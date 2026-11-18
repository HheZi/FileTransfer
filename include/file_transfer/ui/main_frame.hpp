#ifndef UI_MAIN_FRAME_HPP
#define UI_MAIN_FRAME_HPP

#include "file_transfer/client/discovery_handler.hpp"
#include "file_transfer/server/client_connection_handler.hpp"
#include "file_transfer/server/server.hpp"
#include "file_transfer/client/offer_file_command.hpp"
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
  MainFrame(wxString work_dir, wxString name);

  ~MainFrame();

  void startServer();

private:
  boost::asio::io_context io_context_;

  long selected_item_id = -1;

  wxStaticText *wx_text_name{nullptr};
  wxListCtrl *wx_list_connections{nullptr};
  wxButton *wx_send_file_button{nullptr};
  wxProgressDialog *wx_progress_dialog{nullptr};
  std::unique_ptr<wxBusyInfo> wx_busy_info;

  std::string server_id_;

  DiscoveryHandler discovery_handler_;
  std::unique_ptr<Server> server_;

  wxString name_;
  wxString work_dir_;

  std::thread m_server_thread;
  std::thread m_discovery_handler_thread;

  std::unordered_map<std::string, ClientDetails> client_details_holder_;

  void sendFile(const wxString client_id);

  void handleFileProgress(const std::uint64_t total, const std::uint64_t got);

  bool decideToGetAFile(const FileInfo &info);

  void handleFileOfferResult(const std::string path, const OfferResult &result,
                             std::shared_ptr<Connection> connection);

  void handleClientDiscovery(const ClientDetails &details);
};

#endif
