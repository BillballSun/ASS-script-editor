# ASS-script-editor

100% by C standard libarary, without mush function, and neither GUI

to support cross-platform, I used standard scanf and printf to scan the string.

the may entrance is the ASSFile.h, hope you enjoy it.

/* Issue */

1. the Xcode or apple's standard C library seems not support the whole wchar_t range. (Resolved)
How I resolved:
  Unix terminal $local - a
  Then setlocale("zh_CN", LOCAL_ALL);
