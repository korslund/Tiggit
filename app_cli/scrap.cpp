  cout << "\n  " << setw(40) << left << "Title"
        << "   " << setw(10) << "Rating"
        << "   " << setw(10) << "Downloads\n\n";

   const List::PtrList &games = lister.getList();
   for(int i=0; i<games.size(); i++)
     {
       const TigEntry *ent = tt(games[i]);
       std::string title = ent->tigInfo.title;
       if(title.size() > 35)
         title = title.substr(0,35) + "...";

       cout << "  " << setw(40) << left << title
            << "   " << setw(10) << ent->rating
            << "   " << setw(10) << ent->dlCount
            << endl;

       if(i > 30)
         {
           cout << "...\n";
           break;
         }
     }
