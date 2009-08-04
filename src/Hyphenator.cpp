/* libhyphenate: A TeX-like hyphenation algorithm.
 * Copyright (C) 2007 Steve Wolter
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * If you have any questions, feel free to contact me:
 * http://swolter.sdf1.org
 **/
#include "Hyphenator.h"
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <ctype.h>
#include <stdlib.h>

#include <iconv.h>
#include <errno.h>

#include "HyphenationRule.h"
#include "HyphenationTree.h"

#define UTF8_MAX 6

using namespace std;
using namespace RFC_3066;
using namespace Hyphenate;

/** The hyphenation table parser. */
static auto_ptr<HyphenationTree> read_hyphenation_table(const char *filename) {
   ifstream i (filename, fstream::in);
   auto_ptr<HyphenationTree> output(new HyphenationTree());
   output->loadPatterns(i);

   return output;
}

/** Build a hyphenator for the given language. The hyphenation
   *  patterns for the language will loaded from a file named like
   *  the language string or any prefix of it. The file will be
   *  located in the directory given by the environment variable
   *  LIBHYPHENATE_PATH or, if this is empty, in the compiled-in
   *  pattern directory which defaults to 
   *  /usr/local/share/libhyphenate/patterns .
   *
   * \param lang The language for which hyphenation patterns will be
   *             loaded. */
Hyphenate::Hyphenator::Hyphenator(const RFC_3066::Language& lang) {
   setlocale(LC_CTYPE, "");
   string path = "";

   if (getenv("LIBHYPHENATE_PATH")) {
      path = getenv("LIBHYPHENATE_PATH");
   }

#ifdef LIBHYPHENATE_DEFAULT_PATH
   if (path == "")
      path = LIBHYPHENATE_DEFAULT_PATH;
#endif

   path += "/";

   string filename = lang.find_suitable_file(path);
   dictionary = read_hyphenation_table(filename.c_str());
}

/** Build a hyphenator from the patterns in the file provided. */
Hyphenate::Hyphenator::Hyphenator(const char *filename) {
   dictionary = read_hyphenation_table(filename);
}

Hyphenator::~Hyphenator() {}

std::auto_ptr<std::vector<const HyphenationRule*> > 
   Hyphenate::Hyphenator::applyHyphenationRules(CFStringRef word)
{
   return dictionary->applyPatterns(word);
}
