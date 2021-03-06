/*
 * CSE 109
 * Calvin Tong
 * cyt219
 * Program Description: parses the output of the lexer and builds a tree 
 * I have only implemented some of the methods in this program,
 * the methods I have written have a comment on the top of them
 * Program #5
 */

#include "parser.h"
#include <cstring>
#include <string>

const string Parser::ops[] = {"ADD", "SUB", "AND", "DIV", "REM", "ISEQ", "ISGE", "ISGT", "ISLE",
			      "ISLT", "ISNE", "MULT", "OR", "LOADL", "LOADV", "STOREV", "JUMPF",
			      "JUMP", "INSLABEL", "PRINT", "SEQ", "NULLX", "PRINTLN", "PRINTS"};

Parser::Parser(Lexer& lexerx, ostream& outx): lexer(lexerx), out(outx), lindex(1), tindex(1) {
  token = lexer.nextToken();
}

Parser::~Parser() {
}

void Parser::genCode() {
  TreeNode* programNode = program();
  generateCode(programNode);
}

void Parser::gen(TreeNode* node) {
  switch (node->op) {
    case SEQ:
    case NULLX:
      break;
    case LOADL:
    case LOADV:
    case STOREV:
    case JUMPF:
    case JUMP:
      emit(node->op, node->val);
      break;
    case PRINTS:
      emit(node->op, "\"" + node->val + "\"");
      break;
    case INSLABEL:
      emit(node->val);
      break;
    default:
      emit(node->op);
  }
}

void Parser::generateCode(TreeNode* node) {
  if (node != NULL) {
    generateCode(node->leftChild);
    generateCode(node->rightChild);
    gen(node);
  }
}

Parser::TreeNode* Parser::optimize(TreeNode* node) {
    return NULL;
}

void Parser::error(string message) {
  cerr << message << " Found " << token.getLexeme() << " at line " << token.getLine() << " position " << token.getPos() << endl;
  exit(1);
}

void Parser::check(int tokenType, string message) {
  if (token.getType() != tokenType)
    error(message);
}

/*
 * parses a factor
 * @return the node containing the factor tree
 */
Parser::TreeNode* Parser::factor() {
  TreeNode* factornode;
  int tokentype = token.getType();
  
  //check token type
  switch(tokentype)
  {
   case Token::INTLIT:
    factornode = new TreeNode(LOADL, token.getLexeme());
    break;
   case Token::FLOATLIT:
    factornode = new TreeNode(LOADL, token.getLexeme());
    break;
   case Token::IDENT:
    factornode = new TreeNode(LOADV, token.getLexeme());
    break;
   case Token::LPAREN:
    factornode = expression();
    check(Token::RPAREN, "expression has no closing paren");
    break;
  }
  token = lexer.nextToken();
  return factornode;
}

//not written by me
Parser::TreeNode* Parser::term() {
  TreeNode* termNode = factor();
  TreeNode* factorNode;
  int tokenType = token.getType();
  while (tokenType == Token::TIMES || tokenType == Token::DIVIDE || tokenType == Token::REM) {
    token = lexer.nextToken();
    factorNode = factor();
    switch (tokenType) {
      case Token::TIMES:
        termNode = new TreeNode(MULT, termNode, factorNode);
        break;
      case Token::DIVIDE:
        termNode = new TreeNode(DIV, termNode, factorNode);
        break;
      case Token::REM:
        termNode = new TreeNode(REM, termNode, factorNode);
        break;
    }
    tokenType = token.getType();
  }
  return termNode;
}

/*
 * parses an expression
 * @return a pointer to the node containing the expression tree
 */
Parser::TreeNode* Parser::expression() {
  //get initial term
  TreeNode* expNode = term();
  TreeNode* termNode;
  int tokenType = token.getType();
  
  //if there are more terms iterate through
  while(tokenType == Token::PLUS || tokenType == Token::MINUS)
  {
   token = lexer.nextToken();
   termNode = term();
   switch(tokenType)
   {
    case Token::PLUS:
     expNode = new TreeNode(ADD, expNode, termNode);
     break;
    case Token::MINUS:
     expNode = new TreeNode(SUB, expNode, termNode);
     break;
   }
   tokenType = token.getType();
  }
  
  return expNode;
}

Parser::TreeNode* Parser::relationalExpression() {
  return NULL;
}

Parser::TreeNode* Parser::logicalExpression() {
  return NULL;
}

/*
 * parses a set statment
 * @return a pointer to the node containing the set statment tree
 */
Parser::TreeNode* Parser::setStatement() {
  TreeNode* setnode;
  TreeNode* expnode;
  TreeNode* seqnode;
  
  //check to make sure syntax is correct
  string message = "invalid set syntax";
  check(Token::SET, message);
  token = lexer.nextToken();
  check(Token::IDENT, message);
  
  //create the set node
  setnode = new TreeNode(STOREV, token.getLexeme());
  token = lexer.nextToken();
  check(Token::ASSIGN, message);
  token = lexer.nextToken();

  //parse the expression
  expnode = expression();
  seqnode = new TreeNode(SEQ, expnode, setnode);
  return seqnode;
}

/*
 * parses a print expression
 * @return printnode the node containing the print expression tree
 */
Parser::TreeNode* Parser::parsePrintExpression() {
  TreeNode* stringnode;
  TreeNode* printnode;
  
  //check type of token
  int tokenType = token.getType();
  switch(tokenType)
  {
   //if str lit then just return a node with the string and PRINTS 
   case Token::STRLIT:
    stringnode = new TreeNode(PRINTS, token.getLexeme());
    token = lexer.nextToken();
    return stringnode;
   //if ident then load the variable
   case Token::IDENT:
    stringnode = new TreeNode(LOADV, token.getLexeme());
    token = lexer.nextToken();
    break;
   //else it is an expression so call expression method
   default:
    stringnode = expression();
  }
  printnode = new TreeNode(PRINT, NULL, stringnode);
  return printnode;
}

/*
 * parses a print statement
 * @return the node containing the tree with instructions to execute a print
 */
Parser::TreeNode* Parser::printStatement() {
  TreeNode* printstatenode;
  TreeNode* printnode;
  TreeNode* println = new TreeNode(PRINTLN);
  //get the next Token
  token = lexer.nextToken();
  
  //get initial print statement
  printstatenode = parsePrintExpression();
  
  //loop until commas run out and build the tree out
  int tokenType = token.getType();
  while(tokenType == Token::COMMA)
  {
   token = lexer.nextToken();
   printnode = parsePrintExpression();
   printstatenode = new TreeNode(SEQ, printstatenode, printnode);
   tokenType = token.getType();
  }
  printstatenode = new TreeNode(SEQ, printstatenode, println);
  return printstatenode;
}

Parser::TreeNode* Parser::whileStatement() {
  return NULL;
}

Parser::TreeNode* Parser::forStatement() {
  return NULL;
}

Parser::TreeNode* Parser::ifStatement() {
  return NULL;
}

Parser::TreeNode* Parser::switchStatement() {
  return NULL;  
}

//statement was written by femister
Parser::TreeNode* Parser::statement() {
  TreeNode* statement = NULL;
  switch (token.getType()) {
    case Token::SET:
      statement = setStatement();
      break;
    case Token::PRINT:
      statement = printStatement();
      break;
    default:
      error("Unrecognized statement");
      break;
  }
  return statement;
}

/*
 * parses a compound statement
 * @return a pointer to the node containing the compound statement
 */
Parser::TreeNode* Parser::compoundStatement() {
  //get initial statement
  TreeNode* compnode = statement();
  TreeNode* statementnode;
  
  //if more statements exist then iterate until the end of the program
  while(token.getLexeme() != "end")
  {
   statementnode = statement();
   compnode = new TreeNode(SEQ, compnode, statementnode);
  }
  //check that the next word after end is program
  token = lexer.nextToken();
  check(Token::PROGRAM, "invalid ending");

  return compnode;
}

/*
 * entry point to the tree parses the start of the program
 * @return a popinter to the node that is the root of the tree
 */
Parser::TreeNode* Parser::program() {
  TreeNode* compoundnode;
  string message = "invalid program declaration";
  
  //check to see if the program is initialized correct syntax
  check(Token::PROGRAM, message);
  token = lexer.nextToken();
  check(Token::IDENT, message);
  token = lexer.nextToken();

  //build the tree by calling compound statement
  compoundnode = compoundStatement();
  return compoundnode;
}
