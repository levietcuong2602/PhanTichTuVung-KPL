/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#ifndef __ERROR_H__
#define __ERROR_H__

typedef enum {
  ERR_ENDOFCOMMENT,
  ERR_NUMBERTOOLONG,
  ERR_IDENTTOOLONG,
  ERR_INVALIDCHARCONSTANT,
  ERR_INVALIDSYMBOL
} ErrorCode;


#define ERM_ENDOFCOMMENT "End of comment expected!"
#define ERM_IDENTTOOLONG "Identification too long!"
#define ERM_NUMBERTOOLONG "Number too long!"
#define ERM_INVALIDCHARCONSTANT "Invalid const char!"
#define ERM_INVALIDSYMBOL "Invalid symbol!"
#define CATCH_TOOLONG_ERR 1 // Nếu xảy ra lỗi TOO_LONG: 
							// 0-bắt lỗi, thông báo và exit chương trình
							// 1-bỏ qua và lấy với độ dài định sẵn (MAX_IDENT_LEN), bỏ phần phía sau
							// 

void error(ErrorCode err, int lineNo, int colNo);
void warning(ErrorCode err, int lineNo, int colNo);

#endif
