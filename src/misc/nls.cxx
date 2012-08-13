// ----------------------------------------------------------------------------
// nls.cxx
//
// Copyright (C) 2008
//		Stéphane Fillod, F8CFE
//
// Copyright (C) 2011
//		Stelios Bounanos, M0GLD
//
// This file is part of fldigi.
//
// Fldigi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fldigi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with fldigi.  If not, see <http://www.gnu.org/licenses/>.
// ----------------------------------------------------------------------------

#include <config.h>
#include <locale.h>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>

#include "nls.h"
#include "gettext.h"

using namespace std;

#ifndef PATH_MAX
#  define PATH_MAX 4096
#endif

#ifdef __WOE32__
#  include <string>
#  include <fstream>

// TODO: figure out the percentage automatically; hopefully not in a horribly kludgy way
struct lang_def_t ui_langs[] = {
	{ "en", "en_US", "English",     100 },
	{ "fr", "fr_FR", "Fran\347ais", 38 },
	{ "it", "it_IT", "Italiano",    100 },
	{ "es", "es_ES", "Espa\361ol",  97 },
	{ "pl", "pl_PL", "Polish", 97},
	{ NULL, NULL, NULL, 0 }
};

static string get_win32_lang_dir(const char* homedir = NULL)
{
	string lang_fn;
	if (!homedir) {
		if (!(homedir = getenv("USERPROFILE")))
			return lang_fn;
		lang_fn.assign(homedir).append("\\fldigi.files\\");
	}

	return lang_fn.append("lang.txt");
}

int get_ui_lang(const char* homedir)
{
	string lang = get_win32_lang_dir(homedir);

	ifstream in(lang.c_str());
	if (!in)
		return 0;

	string::size_type u = string::npos;
	while (in >> lang) {
		if (lang[0] != '\n' & lang[0] != '#' && (u = lang.find('_')) != string::npos)
			break;
	}
	in.close();

	if (u != string::npos)
		for (lang_def_t* p = ui_langs; p->lang; p++)
			if (lang == p->lang_region)
				return (int)(p - ui_langs);

	return 0;
}

void set_ui_lang(int lang, const char* homedir)
{
	if ((size_t)lang >= sizeof(ui_langs)/sizeof(*ui_langs) - 1)
		return;

	string langfn = get_win32_lang_dir(homedir);
	ofstream f(langfn.c_str());
	if (f) {
		f << "# Autogenerated file, do not edit\r\n"
		  << ui_langs[lang].lang_region << "\r\n";
		f.close();
	}
}
#endif

int setup_nls(void)
{
	static int nls_set_up = 0;
	if (nls_set_up)
		return nls_set_up;

	setlocale (LC_MESSAGES, "");
	setlocale (LC_CTYPE, "C");
	setlocale (LC_TIME, "");
	// setting LC_NUMERIC might break the config read/write routines

	const char* ldir;
	char buf[PATH_MAX];
	if (!(ldir = getenv("FLDIGI_LOCALE_DIR"))) {
		if (getcwd(buf, sizeof(buf) - strlen("/locale") - 1)) {
#ifdef __WOE32__
			int lang = get_ui_lang();
			setenv("LANGUAGE", ui_langs[lang].lang_region, 1);
#endif
			strcpy(buf + strlen(buf), "/locale");
			struct stat s;
			if (stat(buf, &s) != -1 && S_ISDIR(s.st_mode))
				ldir = buf;
			else
				ldir = LOCALEDIR;
		}
	}

	bindtextdomain(PACKAGE, ldir);
	/* fltk-1.1.x only knows about Latin-1 */
	bind_textdomain_codeset(PACKAGE, "ISO-8859-1");
	textdomain(PACKAGE);

	return nls_set_up = 1;
}
