//
//  CFASSFileParsingResult_Macro.h
//  Pods
//
//  Created by Bill Sun on 2019/9/15.
//

#ifndef CFASSFileParsingResult_Macro_h
#define CFASSFileParsingResult_Macro_h

#define PR_INFO(point,  format, ...) \
CFASSFileParsingResultAddWCSFormat(parsingResult, CFASSFileParsingResultLevelInfo, point, format, ## __VA_ARGS__)
#define PR_WARN(point,  format, ...) \
CFASSFileParsingResultAddWCSFormat(parsingResult, CFASSFileParsingResultLevelWarning, point, format, ## __VA_ARGS__)
#define PR_ERROR(point, format, ...) \
CFASSFileParsingResultAddWCSFormat(parsingResult, CFASSFileParsingResultLevelError, point, format, ## __VA_ARGS__)

#endif /* CFASSFileParsingResult_Macro_h */
