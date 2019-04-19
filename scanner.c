/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"

extern int lineNo;
extern int colNo; 
extern int currentChar;
extern CharCode charCodes[];
int state;
int col, line;

// readConstChar
Token *token;
int beginColNo;
int beginLineNo;
char string[MAX_IDENT_LEN + 1];

/***************************************************************/

void skipBlank()
{
  while (currentChar != EOF && charCodes[currentChar] == CHAR_SPACE)
  {
    // đọc kí tự tiếp theo.
    readChar();
  }
}

void skipComment()
{
  while (currentChar != EOF && state != 5)
  {
    readChar();
    switch (charCodes[currentChar])
    {
    case CHAR_TIMES:
      state = 4;
      break;
    case CHAR_RPAR:
      if (state == 4)
      {
        readChar();
        state = 5;
      }
      break;
    default:
      state = 3;
      break;
    }
  }
  // error không đóng comment
  if(state == 3 || state == 4)
  {
    state = 41;
  }
  else
  // đúng: tiếp tục duyệt
    state = 0;
}

Token *readIdentKeyword(void)
{
  Token *token = makeToken(TK_IDENT, lineNo, colNo);
  char string[MAX_IDENT_LEN + 1];
  int isWarning = 0;
  int beginLineNo = lineNo;
  int beginColNo = colNo;
  int i = 0;
  while (currentChar != EOF && (charCodes[currentChar] == CHAR_DIGIT || charCodes[currentChar] == CHAR_LETTER))
  {
    if (i >= MAX_IDENT_LEN)
    {
      if (!CATCH_TOOLONG_ERR)
      {
        error(ERR_IDENTTOOLONG, lineNo, colNo);
      }
      else
        isWarning = 1;
      readChar();
      continue;
    }
    string[i++] = currentChar;
    readChar();
  }
  string[i] = 0;

  // make token, because checkKeyword returns TK_NONE if kw is not type of Keyword,
  // so we make token type is TK_IDENT instead of TK_NONE
  TokenType tokenType = checkKeyword(string);
  if (tokenType == TK_NONE)
    token->tokenType = TK_IDENT;
  else
    token->tokenType = tokenType;
  strcpy(token->string, string);
  if (isWarning)
    warning(ERR_IDENTTOOLONG, beginLineNo, beginColNo);
  return token;
}

Token *readNumber(void)
{
  Token *token = makeToken(TK_NUMBER, lineNo, colNo);
  int isWarning = 0;
  int beginLineNo = lineNo;
  int beginColNo = colNo;
  int i = 0;
  while (currentChar != EOF && charCodes[currentChar] == CHAR_DIGIT)
  {
    if (i >= MAX_NUMBER_LEN)
    {
      if (!CATCH_TOOLONG_ERR)
        error(ERR_NUMBERTOOLONG, lineNo, colNo);
      else
        isWarning = 1;
      readChar();
      continue;
    }
    token->string[i++] = currentChar;
    readChar();
  }
  token->string[i] = 0;
  token->value = atoi(token->string);
  if (isWarning)
    warning(ERR_NUMBERTOOLONG, beginLineNo, beginColNo);
  return token;
}

int readConstChar(void)
{
  beginColNo = colNo;
  beginLineNo = lineNo;

  readChar();
  string[0] = currentChar;
  string[1] = '\0';
  state = 35;

  if (charCodes[currentChar] == CHAR_SINGLEQUOTE)
  {
    readChar();
    if (charCodes[currentChar] != CHAR_SINGLEQUOTE)
    {
      state = 40;
      return state;
    }
  }

  readChar();
  switch (charCodes[currentChar])
  {
  case CHAR_SINGLEQUOTE:
    state = 36;
    return state;
  default:
    state = 40;
    return state;
  }
}

Token *getToken(void)
{
  Token *token;

  if (currentChar == EOF && state != 41)
  {
    state = 37;
  }

  switch (state)
  {
    case 0:
      switch (charCodes[currentChar])
      {
        case CHAR_SPACE: state = 1; break;
        case CHAR_LPAR: state = 2; break;
        case CHAR_LETTER: state = 8; break;
        case CHAR_DIGIT: state = 10; break;
        case CHAR_PLUS: state = 12; break;
        case CHAR_MINUS: state = 13; break;
        case CHAR_TIMES: state = 14; break;
        case CHAR_SLASH: state = 15; break;
        case CHAR_EQ: state = 16; break;
        case CHAR_COMMA: state = 17; break;
        case CHAR_SEMICOLON: state = 18; break;
        case CHAR_PERIOD: state = 19; break;
        case CHAR_GT: state = 22; break;
        case CHAR_LT: state = 25; break;
        case CHAR_EXCLAIMATION: state = 28; break;
        case CHAR_COLON: state = 31; break;
        case CHAR_SINGLEQUOTE: state = 34; break;
        case CHAR_RPAR: state = 39; break;
        default:
          token = makeToken(TK_NONE, lineNo, colNo);
          error(ERR_INVALIDSYMBOL, lineNo, colNo);
          readChar();
          return token;
      }
      return getToken();
    case 1: // space
      skipBlank();
      state = 0;
      return getToken();
    case 2: // (
      readChar();
      switch (charCodes[currentChar])
      {
      // nếu phát hiện (* thì bắt đầu của comment
      // hết comment khi nó gặp *)
      // không đóng comment sẽ in ra lỗi
      case CHAR_TIMES: state = 3; break;
      case CHAR_PERIOD: state = 6; break;
      default: state = 7; break;
      }
      return getToken();
    case 3: // (* comment
      skipComment();
      return getToken();
    case 6: // (.
      state = 0;
      token = makeToken(SB_LSEL, lineNo, colNo);
      return token;
    case 7: // (
      token = makeToken(SB_LPAR, lineNo, colNo - 1);
      // ko read char nua vi neu se doc mat ki tu sau
      return token;
    case 8: // keyword  
      state = 0;
      return readIdentKeyword();
    case 10: // digital
      return readNumber();
    case 12: // +
      state = 0;
      token = makeToken(SB_PLUS, lineNo, colNo);
      readChar();
      return token;
    case 13: // -
      state = 0;
      token = makeToken(SB_MINUS, lineNo, colNo);
      readChar();
      return token;
    case 14: // *
      state = 0;
      token = makeToken(SB_TIMES, lineNo, colNo);
      readChar();
      return token;
    case 15: // /
      state = 0;
      token = makeToken(SB_SLASH, lineNo, colNo);
      readChar();
      return token;
    case 16: // =
      state = 0;
      token = makeToken(SB_EQ, lineNo, colNo);
      readChar();
      return token;
    case 17: // ,
      state = 0;
      token = makeToken(SB_COMMA, lineNo, colNo);
      readChar();
      return token;
    case 18: // ;
      state = 0;
      token = makeToken(SB_SEMICOLON, lineNo, colNo);
      readChar();
      return token;
    case 19: // .
      col = colNo;
      line = lineNo;
      readChar();
      switch (charCodes[currentChar])
      {
      case CHAR_RPAR: state = 20; break;
      default: state = 21; break;
      }
      return getToken();
    case 20: // .)
      token = makeToken(SB_RSEL, line, col);
      readChar();
      return token;
    case 21: // .
      token = makeToken(SB_PERIOD, line, col);
      return token;
    case 22: // >
      readChar();
      switch (charCodes[currentChar])
      {
      case CHAR_EQ: state = 23; break;
      default: state = 24; break;
      }
      return getToken();
    case 23: // >=
      token = makeToken(SB_GE, lineNo, colNo - 1);
      readChar();
      return token;
    case 24: // >
      token = makeToken(SB_GT, lineNo, colNo - 1);
      return token;
    case 25: // <
      readChar();
      switch (charCodes[currentChar])
      {
      case CHAR_EQ: state = 26; break;
      default: state = 27; break;
      }
      return getToken();
    case 26: // <=
      token = makeToken(SB_LE, lineNo, colNo-1);
      readChar();
      return token;
    case 27: // <
      token = makeToken(SB_LT, lineNo, colNo - 1);
      return token;
    case 28: // !
      readChar();
      switch (charCodes[currentChar])
      {
      case CHAR_EQ: state = 29; break;
      default: state = 30; break;
      }
      return getToken();
    case 29: // !=
      token = makeToken(SB_NEQ, lineNo, colNo - 1);
      readChar();
      return token;
    case 30: // ! error
      token = makeToken(TK_NONE, lineNo, colNo - 1);
      error(ERR_INVALIDSYMBOL, lineNo, colNo);
      readChar();
      return token;
    case 31: // :
      readChar();
      switch (charCodes[currentChar])
      {
      case CHAR_EQ: state = 32; break;
      default: state = 33; break;
      }
      return getToken();
    case 32: // :=
      token = makeToken(SB_ASSIGN, lineNo, colNo - 1);
      readChar();
      return token;
    case 33: // :
      token = makeToken(SB_COLON, lineNo, colNo - 1);
      //readChar();
      return token;
    case 34: // '
      // nếu là 4 dấu '''' -> return '
      // và chỉ cho phép bên trong '' là 1 kí tự
      // nếu quá sẽ báo lỗi simple
      readConstChar();
      return getToken();
    case 36:
      token = makeToken(TK_CHAR, beginLineNo, beginColNo);
      strcpy(token->string, string);
      readChar();
      return token;
    case 37: // eof
      return makeToken(TK_EOF, lineNo, colNo); // tạo token eof
    case 39: // )
      state = 0;
      token = makeToken(SB_RPAR, lineNo, colNo);
      readChar();
      return token;
    case 40:
      token = makeToken(TK_NONE, lineNo, colNo);
      error(ERR_INVALIDCHARCONSTANT, lineNo, colNo);
      return token;
    case 41:
      token = makeToken(TK_NONE, lineNo, colNo - 1);
      error(ERR_ENDOFCOMMENT, lineNo, colNo);
      return token;
    default:
      token = makeToken(TK_NONE, lineNo, colNo);
      error(ERR_INVALIDSYMBOL, lineNo, colNo);
      readChar();
      return token;
  }
}

/******************************************************************/

void printToken(Token *token)
{

  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType)
  {
  case TK_NONE:
    printf("TK_NONE\n");
    break;
  case TK_IDENT:
    printf("TK_IDENT(%s)\n", token->string);
    break;
  case TK_NUMBER:
    printf("TK_NUMBER(%s)\n", token->string);
    break;
  case TK_CHAR:
    printf("TK_CHAR(\'%s\')\n", token->string);
    break;
  case TK_EOF:
    printf("TK_EOF\n");
    break;

  case KW_PROGRAM:
    printf("KW_PROGRAM\n");
    break;
  case KW_CONST:
    printf("KW_CONST\n");
    break;
  case KW_TYPE:
    printf("KW_TYPE\n");
    break;
  case KW_VAR:
    printf("KW_VAR\n");
    break;
  case KW_INTEGER:
    printf("KW_INTEGER\n");
    break;
  case KW_CHAR:
    printf("KW_CHAR\n");
    break;
  case KW_ARRAY:
    printf("KW_ARRAY\n");
    break;
  case KW_OF:
    printf("KW_OF\n");
    break;
  case KW_FUNCTION:
    printf("KW_FUNCTION\n");
    break;
  case KW_PROCEDURE:
    printf("KW_PROCEDURE\n");
    break;
  case KW_BEGIN:
    printf("KW_BEGIN\n");
    break;
  case KW_END:
    printf("KW_END\n");
    break;
  case KW_CALL:
    printf("KW_CALL\n");
    break;
  case KW_IF:
    printf("KW_IF\n");
    break;
  case KW_THEN:
    printf("KW_THEN\n");
    break;
  case KW_ELSE:
    printf("KW_ELSE\n");
    break;
  case KW_WHILE:
    printf("KW_WHILE\n");
    break;
  case KW_DO:
    printf("KW_DO\n");
    break;
  case KW_FOR:
    printf("KW_FOR\n");
    break;
  case KW_TO:
    printf("KW_TO\n");
    break;

  case SB_SEMICOLON:
    printf("SB_SEMICOLON\n");
    break;
  case SB_COLON:
    printf("SB_COLON\n");
    break;
  case SB_PERIOD:
    printf("SB_PERIOD\n");
    break;
  case SB_COMMA:
    printf("SB_COMMA\n");
    break;
  case SB_ASSIGN:
    printf("SB_ASSIGN\n");
    break;
  case SB_EQ:
    printf("SB_EQ\n");
    break;
  case SB_NEQ:
    printf("SB_NEQ\n");
    break;
  case SB_LT:
    printf("SB_LT\n");
    break;
  case SB_LE:
    printf("SB_LE\n");
    break;
  case SB_GT:
    printf("SB_GT\n");
    break;
  case SB_GE:
    printf("SB_GE\n");
    break;
  case SB_PLUS:
    printf("SB_PLUS\n");
    break;
  case SB_MINUS:
    printf("SB_MINUS\n");
    break;
  case SB_TIMES:
    printf("SB_TIMES\n");
    break;
  case SB_SLASH:
    printf("SB_SLASH\n");
    break;
  case SB_LPAR:
    printf("SB_LPAR\n");
    break;
  case SB_RPAR:
    printf("SB_RPAR\n");
    break;
  case SB_LSEL:
    printf("SB_LSEL\n");
    break;
  case SB_RSEL:
    printf("SB_RSEL\n");
    break;
  }
}

int scan(char *fileName)
{
  //khởi tạo state = 0
  state = 0;
  // khởi tạo token rỗng
  Token *token;
  // đọc file đầu vào được viết bằng kpl
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;
  
  token = getToken();
  while (token->tokenType != TK_EOF)
  {
    state = 0;
    printToken(token);
    free(token);
    token = getToken();
  }

  free(token);
  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

int main(int argc, char *argv[])
{
  if (argc <= 1)
  {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (scan(argv[1]) == IO_ERROR)
  {
    printf("Can\'t read input file!\n");
    return -1;
  }

  return 0;
}
