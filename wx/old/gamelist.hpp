/* Changes to be made here:

   - ListKeeper might not exist in its current form. Figure out what
     we want it to be from the plan.

   - The GameInfo used in ColumnHandler no longer exists. We need a
     wx-specific counterpart that represents our live data. Thus we
     need a small intermetiary layer between the data model and the wx
     interface, which main task is to cache wxString copies, aplty
     formatted special strings, screenshot data, and other
     display-specific data.

     I want to avoid using an 'extra' pointer if I can. It's better to
     get unique data indices and use an array of pointers which we can
     index directly. If the main data list updates (a rare event), we
     discard the stored wx-data completely.

   - The BIGGEST change is that updates now originate from the data
     model, not the other way around. We will be NOTIFIED when the
     data has changed. We don't manipulate the list keeper itself.

   - So I guess the central system generates the various lists, then
     keeps track of them and update them as necessary, and whenever a
     list knows that it has changed, it updates its user list.
 */
