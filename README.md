# ASS-script-editor

100% by C standard libarary, without mush function, and neither GUI

to support cross-platform, I used standard scanf and printf to scan the string. But the problem is that, the Xcode or apple's standard C library seems not support the whole wchar_t range. So when meet some chinese or korean words, this may process failed.

But seems the VS environment support well.

May later I will rewrite the new version for Cocoa platform, using the NS family.

the may entrance is the ASSFile.h, hope you enjoy it.
