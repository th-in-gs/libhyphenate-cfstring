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

/* This source file provides code for the HyphenationRule class which
 * is documented in HyphenationRule.h */
#include "HyphenationRule.h"

using namespace std;

Hyphenate::HyphenationRule::HyphenationRule(CFStringRef dpattern_string)
: del_pre(0), skip_post(0)
{
   CFIndex dpattern_length = CFStringGetLength(dpattern_string);
   UniChar *dpattern = new UniChar[dpattern_length];
   CFStringGetCharacters(dpattern_string, CFRangeMake(0, dpattern_length), dpattern);
   
   UniChar *key_builder = new UniChar[dpattern_length];
   CFIndex key_builder_index = 0;
   UniChar *insert_pre_builder = new UniChar[dpattern_length]; 
   CFIndex insert_pre_builder_index = 0;
   UniChar *insert_post_builder = new UniChar[dpattern_length]; 
   CFIndex insert_post_builder_index = 0;

   int priority = 0;
   CFIndex i;

   for (i = 0; i < dpattern_length && dpattern[i] != '/'; i++)
      if (dpattern[i] >= '0' && dpattern[i] <= '9')
         priority = 10 * priority + dpattern[i] - '0';
      else {
         key_builder[key_builder_index++] = dpattern[i];
         priorities.push_back(priority);
         priority = 0;
      }

   /* Complete and simplify the array. */
   priorities.push_back(priority);
   while (priorities.back() == 0) priorities.pop_back();

   /* Now check for nonstandard hyphenation. First, parse it. */
   if (i < dpattern_length && dpattern[i] == '/') {
      i += 1;    /* Ignore the /. */

      int field = 1;
      uint start = 0, cut = 0;
      for (; i < dpattern_length; i++) {
         if (field == 1 && dpattern[i] == '=')
            field++;
         else if (field >= 2 && field <= 3 && dpattern[i] == ',')
            field++;
         else if (field == 4 && (dpattern[i] < '0' || dpattern[i] > '9'))
            break;
         else if (field == 1)
            insert_pre_builder[insert_pre_builder_index++] = dpattern[i];
         else if (field == 2)
            insert_post_builder[insert_post_builder_index++] = dpattern[i];
         else if (field == 3)
            start = start * 10 + dpattern[i] - '0';
         else if (field == 4)
            cut = cut * 10 + dpattern[i] - '0';
      }
      if (field < 4) /* There was no fourth field */
         cut = key_builder_index - start;
      if (field < 3)
         start = 1;

      skip_post = cut;
      for (uint j = start; j < start+cut && j < priorities.size(); j++) {
         if (priorities[j-1] % 2 == 1) break;
         del_pre++; skip_post--;
      }
   }

   if(key_builder_index)
      key = CFStringCreateWithCharacters(kCFAllocatorDefault, key_builder, key_builder_index);
   else 
      key = NULL;
   
   if(insert_pre_builder_index) 
      insert_pre = CFStringCreateWithCharacters(kCFAllocatorDefault, insert_pre_builder, insert_pre_builder_index);
   else
      insert_pre = NULL;
   
   if(insert_post_builder_index) 
      insert_post = CFStringCreateWithCharacters(kCFAllocatorDefault, insert_post_builder, insert_post_builder_index);
   else
      insert_post = NULL;
      
   delete[] dpattern;
}

Hyphenate::HyphenationRule::~HyphenationRule()
{
   if(key)
      CFRelease(key);
   
   if(insert_pre)
      CFRelease(insert_pre);
   
   if(insert_post)
      CFRelease(insert_post);
}

pair<CFStringRef, int> Hyphenate::HyphenationRule::create_applied_string(CFStringRef word, CFStringRef hyph) const
{
   CFStringRef intermediateWord = create_applied_string_first(word, hyph);
   pair<CFStringRef, int> ret = create_applied_string_second(intermediateWord);
   CFRelease(intermediateWord);
   return ret;
}

CFStringRef Hyphenate::HyphenationRule::create_applied_string_first(CFStringRef word, CFStringRef hyph) const
{
   CFMutableStringRef ret;
   if(insert_pre) {
      ret = CFStringCreateMutableCopy(kCFAllocatorDefault, 
                                      CFStringGetLength(insert_pre) +
                                      CFStringGetLength(word) +
                                      CFStringGetLength(hyph),
                                      word);
      CFStringAppend(ret, insert_pre);
   } else {
      ret = CFStringCreateMutableCopy(kCFAllocatorDefault, 
                                      CFStringGetLength(word) +
                                      CFStringGetLength(hyph),
                                      word);
   }
   CFStringAppend(ret, hyph);

   return ret;
}

pair<CFStringRef, int> Hyphenate::HyphenationRule::create_applied_string_second(CFStringRef word) const
{
   if(insert_post) {
      CFStringRef ret;
      if(word) {
         CFMutableStringRef mutableRet = CFStringCreateMutable(kCFAllocatorDefault, CFStringGetLength(word) + CFStringGetLength(insert_post));
         CFStringAppend(mutableRet, word);
         CFStringAppend(mutableRet, insert_post);
         ret = mutableRet;
      } else {
         ret = (CFStringRef)CFRetain(insert_post);
      }
      return make_pair(ret, skip_post);
   } else {
      if(word)
         CFRetain(word);
      return make_pair(word, skip_post);
   }
}