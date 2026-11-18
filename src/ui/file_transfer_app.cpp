#include "file_transfer/ui/main_frame.hpp"
#include "file_transfer/ui/file_transfer_app.hpp"

bool FileTransferApp::OnInit() {
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

    auto *frame = new MainFrame(work_dir, name);

    try {
      frame->startServer();
    } catch (...) {
      return false;
    }

    frame->Show();
    return true;
  }
