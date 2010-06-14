/* libhyphenate-cfstring: A TeX-like hyphenation algorithm.
 * Copyright (C) 2007 Steve Wolter 
 *   http://swolter.sdf1.org/
 * Modifications Copyright (C) 2009 - 2010 Things Made Out Of Other Things Ltd.
 *   http://th.ingsmadeoutofotherthin.gs/
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
 **/

#ifndef HYPHENATE_HYPHENATOR_H
#define HYPHENATE_HYPHENATOR_H

#include "Language.h"
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <CoreFoundation/CoreFoundation.h>

#include <iconv.h>

namespace Hyphenate {
   class HyphenationTree;
   class HyphenationRule;

   class Hyphenator {
      private:
	 std::auto_ptr<HyphenationTree> dictionary;
      public:
         /** Build a hyphenator for the given language. The hyphenation
          *  patterns for the language will loaded from a file named like
          *  the language string or any prefix of it. The file will be
          *  located in the directory given by the environment variable
          *  LIBHYPHENATE_PATH or, if this is empty, in the compiled-in
          *  pattern directory which defaults to 
          *  /usr/local/share/libhyphenate-cfstring/patterns .
          *
          * \param lang The language for which hyphenation patterns will be
          *             loaded. */
	 Hyphenator(const RFC_3066::Language& lang); 

         /** Build a hyphenator from the patterns in the file provided. */
	 Hyphenator(const char *filename); 

         /** Destructor. */
	 ~Hyphenator();

         /** Just apply the hyphenation patterns to the word, but don't 
          *  hyphenate anything.
          *
          *  \returns A vector with the same size as the word with a non-NULL
          *           entry for every hyphenation point. */
         std::auto_ptr<std::vector<const HyphenationRule*> > 
            applyHyphenationRules(CFStringRef word);
   };
}

#endif
