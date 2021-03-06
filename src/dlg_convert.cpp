/*
This file is part of the nppcrypt
(http://www.github.com/jeanpaulrichter/nppcrypt)
a plugin for notepad++ [ Copyright (C)2003 Don HO <don.h@free.fr> ]
(https://notepad-plus-plus.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "npp/PluginInterface.h"
#include "npp/Definitions.h"
#include "mdef.h"
#include "dlg_convert.h"
#include "exception.h"
#include "resource.h"
#include <commctrl.h>
#include "help.h"
#include "preferences.h"
#include "messagebox.h"

DlgConvert::DlgConvert(nppcrypt::Options::Convert& opt) : ModalDialog(), options(opt)
{
}

INT_PTR CALLBACK DlgConvert::run_dlgProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        using namespace nppcrypt;

        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_ASCII, BM_SETCHECK, (options.from == Encoding::ascii), 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE16, BM_SETCHECK, (options.from == Encoding::base16), 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE32, BM_SETCHECK, (options.from == Encoding::base32), 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE64, BM_SETCHECK, (options.from == Encoding::base64), 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_ASCII, BM_SETCHECK, (options.to == Encoding::ascii), 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE16, BM_SETCHECK, (options.to == Encoding::base16), 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE32, BM_SETCHECK, (options.to == Encoding::base32), 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE64, BM_SETCHECK, (options.to == Encoding::base64), 0);
        OnSourceChanged(options.from);

        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_UPPERCASE, BM_SETCHECK, options.uppercase, 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINEBREAKS, BM_SETCHECK, options.linebreaks, 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINELENGTH_SPIN, UDM_SETRANGE32, 1, NPPC_MAX_LINE_LENGTH);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINELENGTH_SPIN, UDM_SETBUDDY, (WPARAM)GetDlgItem(_hSelf, IDC_CONVERT_LINELENGTH), 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINELENGTH_SPIN, UDM_SETPOS32, 0, options.linelength);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_LB_WINDOWS, BM_SETCHECK, (options.eol == nppcrypt::EOL::windows), 0);
        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_LB_UNIX, BM_SETCHECK, !(options.eol == nppcrypt::EOL::windows), 0);

        enableOptions((options.to != Encoding::ascii));
        goToCenter();
        return TRUE;
    }
    case WM_COMMAND:
    {
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(_hSelf, IDC_CANCEL);
        }
        switch (HIWORD(wParam))
        {
        case BN_CLICKED:
        {
            switch (LOWORD(wParam))
            {
            case IDC_OK: case IDC_CONVERT_TOCLIPBOARD:
            {
                try {
                    const byte*             pdata;
                    size_t                  data_length;
                    std::basic_string<byte> buffer;

                    updateOptions(); 
                    if (!help::scintilla::getSelection(&pdata, &data_length)) {
                        return TRUE;
                    }

                    nppcrypt::convert(pdata, data_length, buffer, options, preferences.getBase32Alphabet(), preferences.getBase64Alphabet());
                    if (LOWORD(wParam) == IDC_OK) {
                        help::scintilla::replaceSelection(buffer);
                    } else {
                        help::windows::copyToClipboard(buffer);
                    }
                    EndDialog(_hSelf, IDC_OK);
                } catch (std::exception& exc) {
                    msgbox::error(_hSelf, exc.what());
                } catch (...) {
                    msgbox::error(_hSelf, "unknown exception!");
                }
                break;
            }
            case IDC_CANCEL: case IDCANCEL:
            {
                EndDialog(_hSelf, IDC_CANCEL);
                return TRUE;
            }
            case IDC_CONVERT_FROM_ASCII:
            {
                OnSourceChanged(nppcrypt::Encoding::ascii);
                break;
            }
            case IDC_CONVERT_FROM_BASE16:
            {
                OnSourceChanged(nppcrypt::Encoding::base16);
                break;
            }
            case IDC_CONVERT_FROM_BASE32:
            {
                OnSourceChanged(nppcrypt::Encoding::base32);
                break;
            }
            case IDC_CONVERT_FROM_BASE64:
            {
                OnSourceChanged(nppcrypt::Encoding::base64);
                break;
            }
            case IDC_CONVERT_TO_ASCII:
            {
                OnTargetChanged(nppcrypt::Encoding::ascii);
                enableOptions(false);
                break;
            }
            case IDC_CONVERT_TO_BASE16:
            {
                OnTargetChanged(nppcrypt::Encoding::base16);
                enableOptions(true);
                break;
            }
            case IDC_CONVERT_TO_BASE32:
            {
                OnTargetChanged(nppcrypt::Encoding::base32);
                enableOptions(true);
                break;
            }
            case IDC_CONVERT_TO_BASE64:
            {
                OnTargetChanged(nppcrypt::Encoding::base64);
                enableOptions(true);
                break;
            }
            case IDC_CONVERT_LINEBREAKS:
            {
                bool linebreaks = !!::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINEBREAKS, BM_GETCHECK, 0, 0);
                ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LINELENGTH), linebreaks);
                ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LINELENGTH_SPIN), linebreaks);
                ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LB_WINDOWS), linebreaks);
                ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LB_UNIX), linebreaks);
                ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_STATIC1), linebreaks);
                break;
            }
            }
            break;
        }
        case EN_CHANGE:
        {
            /* prevent out of bounds user input to line-length spin-control */
            if (LOWORD(wParam) == IDC_CONVERT_LINELENGTH) {
                int temp_length;
                int len = GetWindowTextLength(::GetDlgItem(_hSelf, IDC_CONVERT_LINELENGTH));
                if (len > 0) {
                    std::wstring temp_str;
                    temp_str.resize(len + 1);
                    ::GetDlgItemText(_hSelf, IDC_CONVERT_LINELENGTH, &temp_str[0], (int)temp_str.size());
                    temp_length = std::stoi(temp_str.c_str());
                    if (temp_length > NPPC_MAX_LINE_LENGTH) {
                        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINELENGTH_SPIN, UDM_SETPOS32, 0, NPPC_MAX_LINE_LENGTH);
                    } else if (temp_length < 1) {
                        ::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINELENGTH_SPIN, UDM_SETPOS32, 0, 1);
                    }
                } else {
                    ::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINELENGTH_SPIN, UDM_SETPOS32, 0, 1);
                }
            }
            break;
        }
        }
        break;
    }
    }
    return FALSE;
}

void DlgConvert::updateOptions()
{
    if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_ASCII, BM_GETCHECK, 0, 0))
        options.from = nppcrypt::Encoding::ascii;
    else if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE16, BM_GETCHECK, 0, 0))
        options.from = nppcrypt::Encoding::base16;
    else if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE32, BM_GETCHECK, 0, 0))
        options.from = nppcrypt::Encoding::base32;
    else if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE64, BM_GETCHECK, 0, 0))
        options.from = nppcrypt::Encoding::base64;

    if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_ASCII, BM_GETCHECK, 0, 0))
        options.to = nppcrypt::Encoding::ascii;
    else if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE16, BM_GETCHECK, 0, 0))
        options.to = nppcrypt::Encoding::base16;
    else if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE32, BM_GETCHECK, 0, 0))
        options.to = nppcrypt::Encoding::base32;
    else if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE64, BM_GETCHECK, 0, 0))
        options.to = nppcrypt::Encoding::base64;

    if (options.to != nppcrypt::Encoding::ascii) {
        options.uppercase = !!::SendDlgItemMessage(_hSelf, IDC_CONVERT_UPPERCASE, BM_GETCHECK, 0, 0);
        options.linebreaks = !!::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINEBREAKS, BM_GETCHECK, 0, 0);
        options.eol = !!::SendDlgItemMessage(_hSelf, IDC_CONVERT_LB_WINDOWS, BM_GETCHECK, 0, 0) ? nppcrypt::EOL::windows : nppcrypt::EOL::unix;
        options.linelength = (int)::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINELENGTH_SPIN, UDM_GETPOS32, 0, 0);
    }
}

void DlgConvert::enableOptions(bool v) const
{
    if (v) {
        bool base64 = !!::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE64, BM_GETCHECK, 0, 0);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_UPPERCASE), !base64);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LINEBREAKS), true);
        bool linebreaks = !!::SendDlgItemMessage(_hSelf, IDC_CONVERT_LINEBREAKS, BM_GETCHECK, 0, 0);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LINELENGTH), linebreaks);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LINELENGTH_SPIN), linebreaks);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LB_WINDOWS), linebreaks);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LB_UNIX), linebreaks);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_STATIC1), linebreaks);
    } else {
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_UPPERCASE), false);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LINEBREAKS), false);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LINELENGTH), false);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LINELENGTH_SPIN), false);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LB_WINDOWS), false);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_LB_UNIX), false);
        ::EnableWindow(::GetDlgItem(_hSelf, IDC_CONVERT_STATIC1), false);
    }
}

void DlgConvert::OnSourceChanged(nppcrypt::Encoding enc) const
{
    switch (enc)
    {
    case nppcrypt::Encoding::ascii:
    {
        if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_ASCII, BM_GETCHECK, 0, 0)) {
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE16, BM_SETCHECK, true, 0);
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_ASCII, BM_SETCHECK, false, 0);
            enableOptions(true);
        }
        break;
    }
    case nppcrypt::Encoding::base16:
    {
        if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE16, BM_GETCHECK, 0, 0)) {
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE32, BM_SETCHECK, true, 0);
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE16, BM_SETCHECK, false, 0);
        }
        break;
    }
    case nppcrypt::Encoding::base32:
    {
        if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE32, BM_GETCHECK, 0, 0)) {
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE64, BM_SETCHECK, true, 0);
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE32, BM_SETCHECK, false, 0);
            enableOptions(true);
        }
        break;
    }
    case nppcrypt::Encoding::base64:
    {
        if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE64, BM_GETCHECK, 0, 0)) {
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE32, BM_SETCHECK, true, 0);
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_TO_BASE64, BM_SETCHECK, false, 0);
        }
        break;
    }
    }
}

void DlgConvert::OnTargetChanged(nppcrypt::Encoding enc) const
{
    switch (enc)
    {
    case nppcrypt::Encoding::ascii:
    {
        if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_ASCII, BM_GETCHECK, 0, 0)) {
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE16, BM_SETCHECK, true, 0);
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_ASCII, BM_SETCHECK, false, 0);
            enableOptions(true);
        }
        break;
    }
    case nppcrypt::Encoding::base16:
    {
        if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE16, BM_GETCHECK, 0, 0)) {
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE32, BM_SETCHECK, true, 0);
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE16, BM_SETCHECK, false, 0);
        }
        break;
    }
    case nppcrypt::Encoding::base32:
    {
        if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE32, BM_GETCHECK, 0, 0)) {
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE64, BM_SETCHECK, true, 0);
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE32, BM_SETCHECK, false, 0);
            enableOptions(true);
        }
        break;
    }
    case nppcrypt::Encoding::base64:
    {
        if (!!::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE64, BM_GETCHECK, 0, 0)) {
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE32, BM_SETCHECK, true, 0);
            ::SendDlgItemMessage(_hSelf, IDC_CONVERT_FROM_BASE64, BM_SETCHECK, false, 0);
        }
        break;
    }
    }
}
