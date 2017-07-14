/***************************************************************
 * Name:      WordclockApp.cpp
 * Purpose:   Code for Application Class
 * Author:     ()
 * Created:   2017-07-14
 * Copyright:  ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "WordclockApp.h"
#include "WordclockMain.h"

IMPLEMENT_APP(WordclockApp);

bool WordclockApp::OnInit()
{
    WordclockDialog* dlg = new WordclockDialog();

    return true;
}
