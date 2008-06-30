#include <libhyphenate/Hyphenator.h>
#include <iostream>

using namespace Hyphenate;
using namespace std;

int main () {
   cout << "The hyphenation of the english word example is: "
        << Hyphenator(RFC_3066::Language("en")).hyphenate("example") << endl;
   return 0;
}
