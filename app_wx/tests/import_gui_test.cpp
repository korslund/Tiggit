#include "importer_gui.hpp"
#include "importer_backend.hpp"
#include <iostream>
#include <boost/filesystem.hpp>
#include "print_dir.hpp"
#include <spread/spread.hpp>
#include <wx/wx.h>
#include "wx/boxes.hpp"

using namespace std;
namespace bf = boost::filesystem;

struct TestApp : wxApp
{
  bool OnInit()
  {
    if(!wxApp::OnInit())
      return false;

    string inpdir = "_inp_full";
    string outdir = "_out_full";
    string log = "_full.log";
    string input = "input_full";
    string spreadOut = "_out5";

    Misc::Logger logger(log);

    bf::remove_all(inpdir);
    bf::remove_all(outdir);
    bf::remove_all(spreadOut);

    Spread::SpreadLib spread(spreadOut, "_tmp5");

    cout << "Copying input directory:\n";
    Spread::JobInfoPtr info;
    Import::copyFiles(input, inpdir, false, &spread, info, logger);

    cout << "\nBEFORE:\n";
    printDir(inpdir);

    if(!wxTiggit::Boxes::ask("Press OK to import " + inpdir + " to " + outdir))
      return false;

    logger.print = true;
    if(ImportGui::importRepoGui(inpdir, outdir, &spread))
      cout << "RETURNED TRUE\n";
    else
      cout << "RETURNED FALSE\n";

    cout << "\nAFTER:\n";
    printDir(inpdir);

    cout << "\nOUTPUT:\n";
    printDir(outdir);

    return false;
  }
};

IMPLEMENT_APP(TestApp)
