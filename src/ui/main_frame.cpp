#include "file_transfer/ui/main_frame.hpp"
#include "file_transfer/client/file_send_command.hpp"
#include "file_transfer/common/id_generator.hpp"
#include "file_transfer/common/file_size_converter.hpp"
#include "file_transfer/server/file_receiver.hpp"
#include <boost/system/system_error.hpp>
#include <memory>
#include <wx/event.h>
#include <wx/wx.h>

MainFrame::MainFrame(wxString work_dir, wxString name)
  : io_context_(), server_id_(IdGenerator::generate()), work_dir_(work_dir),
    name_(name), discovery_handler_(io_context_, server_id_),
    server_(std::make_unique<Server>(work_dir.ToStdString(), server_id_,
				     name.ToStdString())),
    wxFrame(nullptr, wxID_ANY, "File Transfer", wxDefaultPosition,
	    wxSize(FRAME_WIDTH, FRAME_HEIGH)) {
  auto *panel = new wxPanel(this);
  auto *sizer = new wxBoxSizer(wxVERTICAL);

  wx_text_name = new wxStaticText(panel, wxID_ANY, name_, wxDefaultPosition,
				  wxDefaultSize, wxTE_READONLY);
  wx_list_connections = new wxListCtrl(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
				       wxLC_REPORT | wxLC_SINGLE_SEL);
  wx_list_connections->AppendColumn("Id", wxLIST_FORMAT_LEFT, 250);
  wx_list_connections->AppendColumn("Name", wxLIST_FORMAT_LEFT, 250);
  wx_list_connections->AppendColumn("IP", wxLIST_FORMAT_LEFT, 200);

  wx_send_file_button = new wxButton(panel, wxID_ANY, "Send file");
  wx_send_file_button->Enable(false);

  sizer->Add(wx_text_name, 0, wxEXPAND | wxALL, 8);
  sizer->Add(wx_list_connections, 1, wxEXPAND | wxALL, 8);
  sizer->Add(wx_send_file_button, 0, wxALIGN_RIGHT | wxALL, 8);
  panel->SetSizer(sizer);

  wx_list_connections->Bind(wxEVT_LIST_ITEM_SELECTED, [this](wxListEvent &e) {
    wx_send_file_button->Enable(true);
    selected_item_id = e.m_itemIndex;
  });

  wx_list_connections->Bind(wxEVT_LIST_ITEM_DESELECTED,
			    [this](wxListEvent &e) {
			      wx_send_file_button->Enable(false);
			      selected_item_id = -1;
			    });

  wx_send_file_button->Bind(wxEVT_BUTTON, [this](wxCommandEvent &event) {
    if (selected_item_id == -1)
      return;

    this->sendFile(wx_list_connections->GetItemText(selected_item_id));
  });
}

MainFrame::~MainFrame() {
  io_context_.stop();
  if (server_) {
    server_->stop();
  }

  if (m_server_thread.joinable()) {
    m_server_thread.join();
  }
  if (m_discovery_handler_thread.joinable()) {
    m_discovery_handler_thread.join();
  }
}

void MainFrame::startServer() {
  m_server_thread = std::thread([this]() {
    auto decider = [this](const FileInfo &info) {
      return this->decideToGetAFile(info);
    };

    auto file_receiver_handler = [this](std::shared_ptr<FileReceiver> receiver) {
      CallAfter(
		[this, receiver]() { this->handleFileReceive(receiver); });
    };

    server_->setDecider(decider);
    server_->setFileReceiverHandler(file_receiver_handler);

    server_->start();
  });

  m_discovery_handler_thread = std::thread([this]() {
    try {
      auto observer = [this](const ClientDetails &details) {
	CallAfter([this, details]() { handleClientDiscovery(details); });
      };
      discovery_handler_.addObserver(observer);

      discovery_handler_.startReceive();
      io_context_.run();

    } catch (const std::exception &e) {
      std::cerr << "Asio Exception: " << e.what() << std::endl;
    }
  });
}

bool MainFrame::handleFileProgress(const std::uint64_t total, const std::uint64_t got) {
  if (total == 0 || got >= total) {
    if (wx_progress_dialog) {
      wx_progress_dialog->Destroy();
      wx_progress_dialog = nullptr;
    }
    return false;
  }

  if (!wx_progress_dialog) {
    wx_progress_dialog =
        new wxProgressDialog("File Transfer", "Receiving file...", 100, this,
                             wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT |
                                 wxPD_ELAPSED_TIME | wxPD_REMAINING_TIME);
  }

  int percent = static_cast<int>((static_cast<double>(got) / total) * 100.0);
  percent = std::clamp(percent, 0, 100);

  wxString msg = wxString::Format(
      "Received %s of %s bytes (%d%%)", FileSizeConvert::toHumanReadable(got),
      FileSizeConvert::toHumanReadable(total), percent);

  bool continue_transfer = true;
  bool is_canceled = wx_progress_dialog->Update(percent, msg, &continue_transfer);

  if (!is_canceled || !continue_transfer) {
    wx_progress_dialog->Destroy();
    wx_progress_dialog = nullptr;

    return true;
  }

  return false;
}

bool MainFrame::decideToGetAFile(const FileInfo &info) {
  auto promise = std::make_shared<std::promise<bool>>();
  std::future<bool> future = promise->get_future();

  const std::string message =
      "Name: " + info.name +
      "\nSize: " + FileSizeConvert::toHumanReadable(info.size);

  CallAfter([this, message, promise]() {
    wxMessageDialog dlg(this, message, "File offer received",
			wxYES_NO | wxICON_QUESTION);
    int response = dlg.ShowModal();

    promise->set_value(response == wxID_YES);
  });

  return future.get();
}

void MainFrame::sendFile(const wxString client_id) {
  std::string id = client_id.ToStdString();
  if (!client_details_holder_.contains(id)) {
    wxMessageBox("Client not found!", "Error", wxOK | wxICON_ERROR, this);
    return;
  }
  ClientDetails &details = client_details_holder_[id];

  wxFileDialog dlg(this, "Choose file", "", "", "*.*",
		   wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (dlg.ShowModal() != wxID_OK) {
    return;
  }

  std::string path = dlg.GetPath().ToStdString();

  std::shared_ptr<Connection> connection = std::make_shared<Connection>(
      io_context_, details.host, std::to_string(details.port));
  OfferFileCommand command(connection);

  wx_busy_info = std::make_unique<wxBusyInfo>("Waiting for response from receiver...");

  auto handler = [this, path, connection](const OfferResult &result) {
    CallAfter([this, path, result, connection]() {
      handleFileOfferResult(path, result, connection);
    });
  };

  command.offerFile(path, handler);
}

void MainFrame::handleFileOfferResult(const std::string path, const OfferResult &result,
				      std::shared_ptr<Connection> connection) {
  wx_busy_info.reset(nullptr);

  if (!result.file_accepted) {
    wxMessageDialog msg_dialog(this, "File was declined", "Warning",
			       wxOK | wxICON_WARNING);
    msg_dialog.ShowModal();
    return;
  }

  FileSendCommand file_send_cmd(connection, path);

  const std::uint64_t total_size = std::filesystem::file_size(path);

  try {
    for (std::uint64_t bytes_sent : file_send_cmd.sendFile()) {
      if (handleFileProgress(total_size, bytes_sent)) {
	file_send_cmd.cancel();
	return;
      }
      wxYield();
    }
  } catch (boost::system::system_error &e) {
    handleConnectionError();
    return;
  }

  wxMessageDialog msg_dialog(this, "File was sent successfully", "Info",
			     wxOK | wxICON_INFORMATION);
  msg_dialog.ShowModal();
}

void MainFrame::handleClientDiscovery(const ClientDetails &details) {
  long index = wx_list_connections->FindItem(-1, details.id);
  if (index == wxNOT_FOUND) {
    if (details.is_closed)
      return;

    index = wx_list_connections->InsertItem(0, details.id);
  }

  if (details.is_closed) {
    wx_list_connections->DeleteItem(index);
    client_details_holder_.erase(details.id);
    return;
  }

  wx_list_connections->SetItem(index, 1, details.name);
  wx_list_connections->SetItem(index, 2, details.host);

  client_details_holder_[details.id] = details;
}

void MainFrame::handleFileReceive(std::shared_ptr<FileReceiver> receiver) {
  const std::uint64_t total_size = receiver->file_size;

  try {
    for (std::uint64_t bytes_sent : receiver->receiveFile()) {
      if (handleFileProgress(total_size, bytes_sent)) {
        receiver->stop();
        return;
      }
      wxYield();
    }
  } catch (boost::system::system_error &e) {
    handleConnectionError();
    return;
  }

  wxMessageDialog msg_dialog(this, "File was successfully received", "Info",
			     wxOK | wxICON_INFORMATION);
  msg_dialog.ShowModal();
}

void MainFrame::handleConnectionError() {
  if (wx_progress_dialog) {
    wx_progress_dialog->Destroy();
    wx_progress_dialog = nullptr;
  }

  wxMessageDialog msg_dialog(this, "The connection has been closed", "Info",
                             wxOK | wxICON_INFORMATION);
  msg_dialog.ShowModal();
}
