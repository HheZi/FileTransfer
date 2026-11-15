#include "file_transfer/client/connection.hpp"
#include "file_transfer/client/discovery_handler.hpp"
#include "file_transfer/client/file_send_command.hpp"
#include "file_transfer/client/offer_file_command.hpp"
#include "file_transfer/common/id_generator.hpp"
#include "file_transfer/server/client_connection_handler.hpp"
#include "file_transfer/server/server.hpp"
#include <boost/asio/io_context.hpp>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <wx/wx.h>
#include <wx/button.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/generic/textdlgg.h>
#include <wx/listbase.h>
#include <wx/string.h>
#include <wx/valtext.h>
#include <wx/busyinfo.h>
#include <wx/progdlg.h>
#include <wx/listctrl.h>

class MainFrame : public wxFrame {
  constexpr static int FRAME_WIDTH = 740, FRAME_HEIGH = 480;

public:
  MainFrame(wxString work_dir, wxString name)
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
    sizer->Add(wx_send_file_button, 0, wxALIGN_LEFT | wxALL, 8);
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

      sendFile(wx_list_connections->GetItemText(selected_item_id));
    });

    auto observer = [this](const ClientDetails &details) {
      CallAfter([this, details]() { handleClientDiscovery(details); });
    };

    discovery_handler_.addObserver(observer);
  }

  ~MainFrame() {
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

  void startServer() {
    m_server_thread = std::thread([this]() {
      auto decider = [this](const FileInfo &info) {
	return this->decideToGetAFile(info);
      };

      auto file_progress = [this](const std::uint64_t total,
                                  const std::uint64_t got) {
        CallAfter(
            [this, total, got]() { this->handleFileProgress(total, got); });
      };

      server_->setDecider(decider);
      server_->setFileProgress(file_progress);

      server_->start();
    });

    m_discovery_handler_thread = std::thread([this]() {
      try {
        discovery_handler_.startReceive();
        io_context_.run();

      } catch (const std::exception &e) {
        std::cerr << "Asio Exception: " << e.what() << std::endl;
      }
    });
  }

private:
  boost::asio::io_context io_context_;

  long selected_item_id = -1;

  wxStaticText *wx_text_name{nullptr};
  wxListCtrl *wx_list_connections{nullptr};
  wxButton *wx_send_file_button{nullptr};
  wxProgressDialog *wx_progress_dialog {nullptr};

  std::string server_id_;

  DiscoveryHandler discovery_handler_;
  std::unique_ptr<Server> server_;

  wxString name_;
  wxString work_dir_;

  std::thread m_server_thread;
  std::thread m_discovery_handler_thread;

  std::unordered_map<std::string, ClientDetails> client_details_holder_;

  void handleFileProgress(const std::uint64_t total, const std::uint64_t got) {
    if (total == 0 || got >= total) {
      if (wx_progress_dialog) {
        wx_progress_dialog->Destroy();
        wx_progress_dialog = nullptr;
      }
      return;
    }

    if (!wx_progress_dialog) {
      wx_progress_dialog = new wxProgressDialog(
          "File Transfer", "Receiving file...", 100, this,
          wxPD_APP_MODAL | wxPD_AUTO_HIDE | wxPD_CAN_ABORT | wxPD_ELAPSED_TIME |
              wxPD_REMAINING_TIME);
    }

    int percent = static_cast<int>((static_cast<double>(got) / total) * 100.0);
    percent = std::clamp(percent, 0, 100);

    wxString msg =
        wxString::Format("Received %llu of %llu bytes (%d%%)",
                         static_cast<unsigned long long>(got),
                         static_cast<unsigned long long>(total), percent);

    bool continue_transfer = true;
    wx_progress_dialog->Update(percent, msg, &continue_transfer);

    if (!continue_transfer) {
      wx_progress_dialog->Destroy();
      wx_progress_dialog = nullptr;
    }
  }

  bool decideToGetAFile(const FileInfo &info) {
    auto promise = std::make_shared<std::promise<bool>>();
    std::future<bool> future = promise->get_future();

    const std::string message =
        "Name: " + info.name + " Size: " + std::to_string(info.size);

    CallAfter([this, message, promise]() {
      wxMessageDialog dlg(this, message, "File offer received",
                          wxYES_NO | wxICON_QUESTION);
      int response = dlg.ShowModal();

      promise->set_value(response == wxID_YES);
    });

    return future.get();
  }

  void sendFile(const wxString client_id) {
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

    Connection connection(details.host, std::to_string(details.port));
    OfferFileCommand command(connection);

    bool offerAccepted = false;
    {
      wxBusyInfo wait ("Waiting for response from receiver...");

      wxYield();

      offerAccepted = command.offerFile(path);
    }

    if (!offerAccepted) {
      wxMessageDialog msg_dialog(
          this, "File was declined", "Warning",
          wxOK |
              wxICON_WARNING);
      msg_dialog.ShowModal();
      return;
    }

    FileSendCommand file_send_cmd(connection);

    const std::uint64_t total_size = std::filesystem::file_size(path);

    for (std::uint64_t bytes_sent : file_send_cmd.sendFile(path)) {
      handleFileProgress(total_size, bytes_sent);
    }

    wxMessageDialog msg_dialog(this, "File was sent successfully", "Info",
                               wxOK | wxICON_INFORMATION);
    msg_dialog.ShowModal();
  }

  void handleClientDiscovery(const ClientDetails &details) {
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
};

class MyApp : public wxApp {
public:
  bool OnInit() override {
    wxString name;
    wxTextEntryDialog name_dialog(nullptr, "Enter the name", "Name", "App");
    name_dialog.SetTextValidator(wxTextValidator(wxFILTER_NONE, &name));

    if (name_dialog.ShowModal() != wxID_OK) {
      return false;
    }

    std::string work_dir;
    wxDirDialog dirDlg(nullptr, "Choose Working Directory", "",
                       wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dirDlg.ShowModal() == wxID_OK) {
      work_dir = dirDlg.GetPath();
    } else {
      return false;
    }

    auto *f = new MainFrame(work_dir, name);

    try {
      f->startServer();
    } catch (...) {
      return false;
    }

    f->Show();
    return true;
  }
};

wxIMPLEMENT_APP(MyApp);
