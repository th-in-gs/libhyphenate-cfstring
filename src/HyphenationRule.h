/* libhyphenate: A TeX-like hyphenation algorithm.
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
#ifndef HYPHENATION_RULE_H
#define HYPHENATION_RULE_H

#include <string>
#include <vector>
#include <CoreFoundation/CoreFoundation.h>

namespace Hyphenate {
   /** The HyphenationRule class represents a single Hyphenation Rule, that
    *  is, a pattern that has a number assigned to each letter and will,
    *  if applied, hyphenate a word at the given point. The number assigned
    *  to each letter and accessed by priority() is odd when hyphenation
    *  should occur before the letter, and only the rule with the highest
    *  number will be applied to any letter. */
   class HyphenationRule {
      private:
         int del_pre, skip_post;
         CFStringRef key, insert_pre, insert_post;
         std::vector<char> priorities;

         std::string replacement;

      public:
         /* HyphenationRule is constructed from a string consisting of
          * letters with numbers strewn in. The numbers are the priorities.
          * In addition, a / will start a non-standard hyphenization. */
         HyphenationRule(CFStringRef source_string);
         ~HyphenationRule();
      
         /** Call this method once an hyphen would, according to its base rule,
         *   be placed. Returns the number of bytes that should not be  
         *   printed afterwards.
         *
         *   For example, when applying the rules to "example", you should
         *   call the rules returned by HyphenationTree or Hyphenator as
         *   follows:
         *   string word = "ex";
         *   rule1.apply(word, "-");
         *   word += "am" ;
         *   rule2.apply(word, "-");
         *   word += "ple";
         *
         *   Watch out for non-standard rules, though. Example: "Schiffahrt"
         *   string word = "Schif";
         *   int skip = rule1.apply(word, "-");
         *   char *rest = "fahrt";
         *   word += rest+skip;
         */
         std::pair<CFStringRef, int> create_applied_string(CFStringRef word, CFStringRef hyphen) const;
         /** Only apply the first part, that is, up to and including the
          *  hyphen. */
         CFStringRef create_applied_string_first(CFStringRef word, CFStringRef hyphen) const;
         /** Only apply the second part, after the hyphen. */
         std::pair<CFStringRef, int> create_applied_string_second(CFStringRef word) const;

         /** Returns true iff there is a priority value != 0 for this offset
          *  or a larger one. */
         inline bool hasPriority(uint offset) const 
            { return priorities.size() > offset; }
         /** Returns the hyphenation priority for a hyphen preceding the byte
          *  at the given offset. */
         inline char priority(uint offset) const { return priorities[offset]; }

         /** Returns the pattern to match for this rule to apply. */
         inline CFStringRef getKey() { return key; }

         /** Returns the amount of bytes that will additionally be needed
          *  in front of the hyphen if this rule is applied. 0 for standard
          *  hyphenation, 1 for Schiff-fahrt. */
         int spaceNeededPreHyphen() const 
      { return (insert_pre ? CFStringGetLength(insert_pre) : 0) - del_pre; }
         
         /** Returns true iff this rule is not a standard hyphenation rule. */
         bool isNonStandard() const
            { return del_pre != 0 || skip_post != 0 || insert_pre || insert_post; }
   };
}

#endif
