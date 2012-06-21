  std::vector<TagSorter::Entry> taglist;

  void createTagList()
  {
    // Set up the tag list
    const vector<int> &base = lister.getBaseList();
    tagSorter.makeTagList(base, taglist);

    // Create and set up the wxString version of the tag list
    std::vector<wxString> labels;
    labels.resize(taglist.size()+1);
    labels[0] = wxString::Format(wxT("All (%d)"), base.size());
    for(int i=0; i<taglist.size(); i++)
      {
        TagSorter::Entry &e = taglist[i];
        wxString &str = labels[i+1];

        str = wxString(e.tag.c_str(), wxConvUTF8);
        str += wxString::Format(wxT(" (%d)"), e.games.size());
      }

    // Clear tag control
    tags->Clear();

    // Add new strings to it
    tags->InsertItems(labels.size(), &labels[0], 0);
  }
