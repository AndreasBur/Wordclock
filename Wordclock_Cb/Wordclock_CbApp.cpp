/***************************************************************
 * Name:      Wordclock_CbApp.cpp
 * Purpose:   Code for Application Class
 * Author:     ()
 * Created:   2017-07-11
 * Copyright:  ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "Wordclock_CbApp.h"
#include "Wordclock_CbMain.h"

IMPLEMENT_APP(Wordclock_CbApp);

bool Wordclock_CbApp::OnInit()
{
    
    Wordclock_CbDialog* dlg = new Wordclock_CbDialog(0L, _("wxWidgets Application Template"));
    dlg->SetIcon(wxICON(aaaa)); // To Set App Icon
    dlg->Show();
    return true;
}
