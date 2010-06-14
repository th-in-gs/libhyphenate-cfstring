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


/* ------------- Implementation for HyphenationTree.h ---------------- */

#include "HyphenationTree.h"
#include <iostream>
#include <stdexcept>
#include <algorithm>

using namespace std;
using namespace Hyphenate;

/* The HyphenationNode is a tree node for the hyphenation search tree. It
* represents the matching state after a single character; if there is a
* pattern that ends with that particular character, the hyphenation_pattern
* is set to non-NULL. The jump_table links to the children of that node,
* indexed by letters. */
class Hyphenate::HyphenationNode {
   public:
      typedef std::map<UniChar, HyphenationNode*> JumpMap;
      /* Table of children */
      HyphenationNode* jump_table[26];
      JumpMap *jump_map;
      /* Hyphenation pattern associated with the full path to this node. */
      std::auto_ptr<HyphenationRule> hyphenation_pattern;

      HyphenationNode() : jump_map(NULL) {
	 memset((void *)jump_table, 0, 26 * sizeof(HyphenationNode*));
      }
	 
      ~HyphenationNode() {
         /* The destructor has to destroy all childrens. */
	 for (int i = 0; i < 26; ++i) {
	    if(jump_table[i]) {
	       delete jump_table[i];
	    }
	 }
	 if(jump_map) {
	    for (JumpMap::iterator i = jump_map->begin();
		  i != jump_map->end(); i++)
	       delete i->second;
	    delete jump_map;
	 }
      }
   
      /** Find a particular jump table entry, or NULL if there is none 
        * for that letter. */
      inline const HyphenationNode *find(UniChar arg) const {
	 if(arg >= 'a' && arg <= 'z') {
	    return jump_table[arg - 'a'];
	 } else if(jump_map) {
	    JumpMap::iterator i = jump_map->find(arg);
	    if (i != jump_map->end()) return i->second; else return NULL;
	 } else {
	    return NULL;
	 }
      }
      
      /** Find a particular jump table entry, or NULL if there is none 
        * for that letter. */
      inline HyphenationNode *find(UniChar arg) {
	 if(arg >= 'a' && arg <= 'z') {
	    return jump_table[arg - 'a'];
	 } else if(jump_map) {
	    JumpMap::iterator i = jump_map->find(arg);
	    if (i != jump_map->end()) return i->second; else return NULL;
	 } else {
	    return NULL;
	 }
      }
   
      /** Insert a particular hyphenation pattern into this 
         *  hyphenation subtree.
      * \param pattern The character pattern to match in the input word.
      * \param hp The digit-pattern for the hyphenation algorithm.
      */
      void insert (const UniChar *id, 
         std::auto_ptr<HyphenationRule> pattern);

      /** Apply all patterns for that subtree. */
      void apply_patterns(
         char *priority_buffer, 
         const HyphenationRule ** rule_buffer, 
         UniChar *to_match) const;
};

Hyphenate::HyphenationTree::HyphenationTree() : 
   root(new HyphenationNode()), start_safe(1), end_safe(1) {
      non_lower_case_letter_characte_set = CFCharacterSetCreateInvertedSet(kCFAllocatorDefault, CFCharacterSetGetPredefined(kCFCharacterSetLowercaseLetter));
}

Hyphenate::HyphenationTree::~HyphenationTree() {
   delete root;
   CFRelease(non_lower_case_letter_characte_set);
}

void Hyphenate::HyphenationTree::insert(auto_ptr<HyphenationRule> pattern) {
   /* Convert our key to lower case to ease matching. */
   CFStringRef upperCaseKey = pattern->getKey();
   CFIndex length = CFStringGetLength(upperCaseKey);
   CFMutableStringRef lowercaseKey = CFStringCreateMutableCopy(kCFAllocatorDefault, length, upperCaseKey);
   CFStringLowercase(lowercaseKey, NULL);
   UniChar *lowercaseKeyCharacters = new UniChar[length  + 1];
   CFStringGetCharacters(lowercaseKey, CFRangeMake(0, length), lowercaseKeyCharacters);
   lowercaseKeyCharacters[length] = 0;
   CFRelease(lowercaseKey);

   root->insert(lowercaseKeyCharacters, pattern);
   delete[] lowercaseKeyCharacters;
}

void HyphenationNode::insert (const UniChar* key_characters, 
                              auto_ptr<HyphenationRule> pattern) 
{
   /* Is this the terminal node for that pattern? */
   if (key_characters[0] == 0) {
      /* If we descended the tree all the way to the last letter, we can now
       * write the pattern into this node. */

      hyphenation_pattern.reset(pattern.release());
   } else  {
      /* If not, however, we make sure that the branch for our letter exists
       * and descend. */
      UniChar key = key_characters[0];
      /* Ensure presence of a branch for that letter. */
      HyphenationNode *p = find(key);
      if (!p) {
	 p = new HyphenationNode();
	 if(key >= 'a' && key <= 'z') {
	    jump_table[key - 'a'] = p;
	 } else {
	    if(!jump_map)
	       jump_map = new JumpMap();
	    jump_map->insert(make_pair(key, p));
	 }
      }
      /* Go to the next letter and descend. */
      p->insert(key_characters+1, pattern);
   }
}

void Hyphenate::HyphenationNode::apply_patterns(
   char *priority_buffer, 
   const HyphenationRule ** rule_buffer, 
   UniChar *to_match) const
{
   /* First of all, if we can descend further into the tree (that is,
    * there is an input char left and there is a branch in the tree),
    * do so. */
   UniChar key = to_match[0];

   if (key != 0) {
      const HyphenationNode *next = find(key);
      if ( next != NULL )
         next->apply_patterns(priority_buffer, rule_buffer, to_match+1);
   }

   /* Now, if we have a pattern at this point in the tree, it must be a good
    * match. Apply the pattern. */
   const HyphenationRule* hyp_pat = hyphenation_pattern.get();
   if (hyp_pat != NULL)
      for (int i = 0; hyp_pat->hasPriority(i); i++)
	 if (priority_buffer[i] < hyp_pat->priority(i)) {
            rule_buffer[i] = (hyp_pat->priority(i) % 2 == 1) ? hyp_pat : NULL;
            priority_buffer[i] = hyp_pat->priority(i);
         }
}

auto_ptr<vector<const HyphenationRule*> > HyphenationTree::applyPatterns
   (CFStringRef word) const
{
   return applyPatterns(word, INT_MAX);
}

auto_ptr<vector<const HyphenationRule*> > HyphenationTree::applyPatterns
   (CFStringRef word, CFIndex stop_at) const
{
   /* Prepend and append a . to the string (word start and end), and convert
    * all characters to lower case to ease matching. */   

   CFCharacterSetRef upperCaseLetterCharacterSet = CFCharacterSetGetPredefined(kCFCharacterSetUppercaseLetter);
   CFMutableStringRef lowerCaseStringToRelease;
   CFRange foundRange;
   CFIndex wordLength = CFStringGetLength(word);
   if(CFStringFindCharacterFromSet(word, upperCaseLetterCharacterSet, CFRangeMake(0, wordLength), 0, &foundRange)) {
      lowerCaseStringToRelease = CFStringCreateMutableCopy(kCFAllocatorDefault, wordLength, word);
      CFStringLowercase(lowerCaseStringToRelease, NULL);
      word = lowerCaseStringToRelease;
   } else {
      lowerCaseStringToRelease = NULL;
   }
      
   CFIndex w_size = wordLength + 2;
   UniChar *characters = new UniChar[w_size + 1];
   characters[0] = '.';
   CFStringGetCharacters(word, CFRangeMake(0, wordLength), characters + 1);
   characters[wordLength + 1] = '.';
   characters[wordLength + 2] = 0;
   
   /* Arrays for priorities and rules. */
   char *pri = (char *)calloc(w_size + 2, sizeof(char));
   const HyphenationRule **rules = (const HyphenationRule **)calloc(w_size + 3, sizeof(HyphenationRule *));
    
   /* For each suffix of the expanded word, search all matching prefixes.
    * That way, each possible match is found. Note the pointer arithmetics
    * in the first and second argument. */
   for (CFIndex i = 0; i < w_size-1 && i <= stop_at; i++)
      root->apply_patterns((&pri[i]), (&rules[i]), characters + i);

   free(pri);
   
   /* Copy the results to a shorter vector. */
   auto_ptr<vector<const HyphenationRule*> > output_rules(
      new vector<const HyphenationRule*>(wordLength, NULL));
   
   /* We honor the safe areas at the start and end of each word here. */
   /* Please note that the incongruence between start and end is due
    * to the fact that hyphenation happens _before_ each character. */
   uint ind_start = 1 + start_safe, ind_end = w_size - 1 - end_safe;
   
   for (uint i = ind_start; i <= ind_end; i++)
      (*output_rules)[i - 1] = rules[i];
   
   /* Remove any hyphens within the safe-distance of punctuation */
   CFRange searchRange = CFRangeMake(0, wordLength);
   while (CFStringFindCharacterFromSet(word, non_lower_case_letter_characte_set, searchRange, 0, &foundRange)) {
      CFIndex i = max(searchRange.location, foundRange.location - start_safe);
      CFIndex upTo = min(wordLength, foundRange.location + foundRange.length + end_safe);
      for (; i < upTo; ++i) {
         (*output_rules)[i] = NULL;
      }
      if(upTo == wordLength) {
         // May as well break, we've zeroed out th the end of the word.
         break;
      }
      searchRange.location = searchRange.location + searchRange.length;
      searchRange.length = wordLength - searchRange.location;
   }
   
   
   free(rules);
   
   if(lowerCaseStringToRelease) 
      CFRelease(lowerCaseStringToRelease);
   
   delete[] characters;
   
   return output_rules;
}

void HyphenationTree::loadPatterns(istream &i) {
   string pattern;
   /* The input is a file with whitespace-separated words.
    * The first numerical-only word we encountered denotes the safe start,
    * the second the safe end area. */

   char ch;
   bool numeric = true;
   int num_field = 0;
   while ( i.get(ch) ) {
      if (ch == '\n' || ch == '\r' || ch == '\t' || ch == ' ') {
	 /* The output operation. */
         if (pattern.size() && numeric && num_field <= 1) {
            ((num_field == 0) ? start_safe : end_safe) = atoi(pattern.c_str());
            num_field++;
	 } else if (pattern.size()) {
	    CFStringRef patternString = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault, (UInt8 *)pattern.c_str(), pattern.size(), kCFStringEncodingUTF8, false, kCFAllocatorNull);
            insert(
               auto_ptr<HyphenationRule>(new HyphenationRule(patternString)));
	    CFRelease(patternString);
         }

	 /* Reinitialize state. */
	 pattern.clear();
         numeric = true;
      } else {
	 /* This rule catches all other (mostly alpha, but probably UTF-8)
	  * characters. It normalizes the previous letter and then appends
          * it to the pattern. */
         pattern += ch;
         if (ch < '0' || ch > '9') numeric = false;
      }
   }

   if (pattern.size())  {
      CFStringRef patternString = CFStringCreateWithBytesNoCopy(kCFAllocatorDefault, (UInt8 *)pattern.c_str(), pattern.size(), kCFStringEncodingUTF8, false, kCFAllocatorNull);
      insert(
	     auto_ptr<HyphenationRule>(new HyphenationRule(patternString)));
      CFRelease(patternString);
   }
}

