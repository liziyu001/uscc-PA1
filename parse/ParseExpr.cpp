//
//  ParseExpr.cpp
//  uscc
//
//  Implements all of the recursive descent parsing
//  functions for the expression grammar rules.
//
//---------------------------------------------------------
//  Copyright (c) 2014, Sanjay Madhav
//  All rights reserved.
//
//  This file is distributed under the BSD license.
//  See LICENSE.TXT for details.
//---------------------------------------------------------

#include "Parse.h"
#include "Symbols.h"
#include <iostream>
#include <sstream>

using namespace uscc::parse;
using namespace uscc::scan;

using std::shared_ptr;
using std::make_shared;

shared_ptr<ASTExpr> Parser::parseExpr()
{
	shared_ptr<ASTExpr> retVal;
	
	// We should first get a AndTerm
	shared_ptr<ASTExpr> andTerm = parseAndTerm();
	
	// If we didn't get an andTerm, then this isn't an Expr
	if (andTerm)
	{
		retVal = andTerm;
		// Check if this is followed by an op (optional)
		shared_ptr<ASTLogicalOr> exprPrime = parseExprPrime(retVal);
		
		if (exprPrime)
		{
			// If we got a exprPrime, return this instead of just term
			retVal = exprPrime;
		}
	}
	
	return retVal;
}

shared_ptr<ASTLogicalOr> Parser::parseExprPrime(shared_ptr<ASTExpr> lhs)
{
	shared_ptr<ASTLogicalOr> retVal;
	
	// Must be ||
	if (peekToken() == Token::Or)
	{
		// Make the binary cmp op
		Token::Tokens op = peekToken();
		retVal = make_shared<ASTLogicalOr>();
		consumeToken();
		
		// Set the lhs to our parameter
		retVal->setLHS(lhs);
		
		// We MUST get a AndTerm as the RHS of this operand
		shared_ptr<ASTExpr> rhs = parseAndTerm();
		if (!rhs)
		{
			throw OperandMissing(op);
		}
		
		retVal->setRHS(rhs);
		
		// PA2: Finalize op
		
		// See comment in parseTermPrime if you're confused by this
		shared_ptr<ASTLogicalOr> exprPrime = parseExprPrime(retVal);
		if (exprPrime)
		{
			retVal = exprPrime;
		}
	}
	
	return retVal;
}

// AndTerm -->
shared_ptr<ASTExpr> Parser::parseAndTerm()
{
	shared_ptr<ASTExpr> retVal;

	shared_ptr<ASTExpr> relExpr = parseRelExpr();
	
	if (relExpr)
	{
		retVal = relExpr;
		shared_ptr<ASTLogicalAnd> andTermPrime = parseAndTermPrime(retVal);
		
		if (andTermPrime)
		{
			retVal = andTermPrime;
		}
	}
	
	return retVal;
}

shared_ptr<ASTLogicalAnd> Parser::parseAndTermPrime(shared_ptr<ASTExpr> lhs)
{
	shared_ptr<ASTLogicalAnd> retVal;

	Token::Tokens op = peekToken();
	if (op == Token::And)
	{
		retVal = make_shared<ASTLogicalAnd>();
		consumeToken();
		
		// Set the lhs to our parameter
		retVal->setLHS(lhs);
		
		shared_ptr<ASTExpr> rhs = parseRelExpr();
		if (!rhs)
		{
			throw OperandMissing(op);
		}
		
		retVal->setRHS(rhs);
		
		shared_ptr<ASTLogicalAnd> andTermPrime = parseAndTermPrime(retVal);
		if (andTermPrime)
		{
			retVal = andTermPrime;
		}
	}
	
	return retVal;
}

// RelExpr -->
shared_ptr<ASTExpr> Parser::parseRelExpr()
{
	shared_ptr<ASTExpr> retVal;
	
	shared_ptr<ASTExpr> numExpr = parseNumExpr();
	
	if (numExpr)
	{
		retVal = numExpr;
		shared_ptr<ASTBinaryCmpOp> relExprPrime = parseRelExprPrime(retVal);
		
		if (relExprPrime)
		{
			retVal = relExprPrime;
		}
	}
	
	return retVal;
}

shared_ptr<ASTBinaryCmpOp> Parser::parseRelExprPrime(shared_ptr<ASTExpr> lhs)
{
	shared_ptr<ASTBinaryCmpOp> retVal;
	
	Token::Tokens op = peekToken();
	if (op == Token::EqualTo || op == Token::NotEqual || op == Token::LessThan || op == Token::GreaterThan)
	{
		retVal = make_shared<ASTBinaryCmpOp>(op);
		consumeToken();
		
		// Set the lhs to our parameter
		retVal->setLHS(lhs);
		
		shared_ptr<ASTExpr> rhs = parseNumExpr();
		if (!rhs)
		{
			throw OperandMissing(op);
		}
		
		retVal->setRHS(rhs);
		
		shared_ptr<ASTBinaryCmpOp> relExprPrime = parseRelExprPrime(retVal);
		if (relExprPrime)
		{
			retVal = relExprPrime;
		}
	}
	
	return retVal;
}

// NumExpr -->
shared_ptr<ASTExpr> Parser::parseNumExpr()
{
	shared_ptr<ASTExpr> retVal;
	
	shared_ptr<ASTExpr> term = parseTerm();
	
	if (term)
	{
		retVal = term;
		shared_ptr<ASTBinaryMathOp> numExprPrime = parseNumExprPrime(retVal);
		
		if (numExprPrime)
		{
			retVal = numExprPrime;
		}
	}
	
	return retVal;
}

shared_ptr<ASTBinaryMathOp> Parser::parseNumExprPrime(shared_ptr<ASTExpr> lhs)
{
	shared_ptr<ASTBinaryMathOp> retVal;

	Token::Tokens op = peekToken();
	if (op == Token::Plus || op == Token::Minus)
	{
		retVal = make_shared<ASTBinaryMathOp>(op);
		consumeToken();
		
		// Set the lhs to our parameter
		retVal->setLHS(lhs);
		
		shared_ptr<ASTExpr> rhs = parseTerm();
		if (!rhs)
		{
			throw OperandMissing(op);
		}
		
		retVal->setRHS(rhs);
		
		shared_ptr<ASTBinaryMathOp> numExprPrime = parseNumExprPrime(retVal);
		if (numExprPrime)
		{
			retVal = numExprPrime;
		}
	}
	
	return retVal;
}

// Term -->
shared_ptr<ASTExpr> Parser::parseTerm()
{
	shared_ptr<ASTExpr> retVal;

	shared_ptr<ASTExpr> value = parseValue();
	
	if (value)
	{
		retVal = value;
		// Check if this is followed by an op (optional)
		shared_ptr<ASTBinaryMathOp> termPrime = parseTermPrime(retVal);
		
		if (termPrime)
		{
			retVal = termPrime;
		}
	}
	
	return retVal;
}

shared_ptr<ASTBinaryMathOp> Parser::parseTermPrime(shared_ptr<ASTExpr> lhs)
{
	shared_ptr<ASTBinaryMathOp> retVal;

	Token::Tokens op = peekToken();
	if (op == Token::Mult || op == Token::Div || op == Token::Mod)
	{
		retVal = make_shared<ASTBinaryMathOp>(op);
		consumeToken();
		
		// Set the lhs to our parameter
		retVal->setLHS(lhs);
		
		// We MUST get a Term as the RHS of this operand
		shared_ptr<ASTExpr> rhs = parseValue();
		if (!rhs)
		{
			throw OperandMissing(op);
		}
		
		retVal->setRHS(rhs);
		
		shared_ptr<ASTBinaryMathOp> termPrime = parseTermPrime(retVal);
		if (termPrime)
		{
			retVal = termPrime;
		}
	}
	
	return retVal;
}

// Value -->
shared_ptr<ASTExpr> Parser::parseValue()
{
	shared_ptr<ASTExpr> retVal;
	shared_ptr<ASTExpr> factor;
	
	if (peekAndConsume(Token::Not)) {
		factor = parseFactor();
		if (!factor) {
			throw ParseExceptMsg("! must be followed by an expression.");
		}
		retVal = make_shared<ASTNotExpr>(factor);
	} else {
		retVal = parseFactor();
	}
	
	return retVal;
}

// Factor -->
shared_ptr<ASTExpr> Parser::parseFactor()
{
	shared_ptr<ASTExpr> retVal;
	
	// Try parse identifier factors FIRST so
	// we make sure to consume the mUnusedIdents
	// before we try any other rules
	if ((retVal = parseIdentFactor()))
		;
	else if ((retVal = parseAddrOfArrayFactor()))
		;
	else if ((retVal = parseParenFactor()))
		;
	else if ((retVal = parseConstantFactor()))
		;
	else if ((retVal = parseStringFactor()))
		;
	else if ((retVal = parseIncFactor()))
		;
	else if ((retVal = parseDecFactor()))
		;
	
	return retVal;
}

// ( Expr )
shared_ptr<ASTExpr> Parser::parseParenFactor()
{
	shared_ptr<ASTExpr> retVal;

	if (peekAndConsume(Token::LParen)) {
		retVal = parseExpr();
		if (!retVal) {
			throw ParseExceptMsg("Not a valid expression inside parenthesis");
		}
		matchToken(Token::RParen);
	}
	
	return retVal;
}

// constant
shared_ptr<ASTConstantExpr> Parser::parseConstantFactor()
{
	shared_ptr<ASTConstantExpr> retVal;

	if (peekToken() == Token::Constant) {
		const char* value = getTokenTxt();	
		retVal = make_shared<ASTConstantExpr>(value);
		consumeToken();
	}
	
	return retVal;
}

// string
shared_ptr<ASTStringExpr> Parser::parseStringFactor()
{
	shared_ptr<ASTStringExpr> retVal;

	if (peekToken() == Token::String) {
		const char* value = getTokenTxt();
		retVal = make_shared<ASTStringExpr>(value, mStrings);
		consumeToken();
	}
	
	return retVal;
}

// id
// id [ Expr ]
// id ( FuncCallArgs )
shared_ptr<ASTExpr> Parser::parseIdentFactor()
{
	shared_ptr<ASTExpr> retVal;
	if (peekToken() == Token::Identifier ||
		mUnusedIdent != nullptr || mUnusedArray != nullptr)
	{
		if (mUnusedArray)
		{
			// "unused array" means that AssignStmt looked at this array
			// and decided it didn't want it, so it's already made an
			// array sub node
			retVal = make_shared<ASTArrayExpr>(mUnusedArray);
			mUnusedArray = nullptr;
		}
		else
		{
			Identifier* ident = nullptr;
			
			// If we have an "unused identifier," which means that
			// AssignStmt looked at this and decided it didn't want it,
			// that means we're already a token AFTER the identifier.
			if (mUnusedIdent)
			{
				ident = mUnusedIdent;
				mUnusedIdent = nullptr;
			}
			else
			{
				ident = getVariable(getTokenTxt());
				consumeToken();
			}
			
			// Now we need to look ahead and see if this is an array
			// or function call reference, since id is a common
			// left prefix.
			if (peekToken() == Token::LBracket)
			{
				// Check to make sure this is an array
				if (mCheckSemant && ident->getType() != Type::IntArray &&
					ident->getType() != Type::CharArray &&
					!ident->isDummy())
				{
					std::string err("'");
					err += ident->getName();
					err += "' is not an array";
					reportSemantError(err);
					consumeUntil(Token::RBracket);
					if (peekToken() == Token::EndOfFile)
					{
						throw EOFExcept();
					}
					
					matchToken(Token::RBracket);
					
					// Just return our error variable
					retVal = make_shared<ASTIdentExpr>(*mSymbols.getIdentifier("@@variable"));
				}
				else
				{
					consumeToken();
					try
					{
						shared_ptr<ASTExpr> expr = parseExpr();
						if (!expr)
						{
							throw ParseExceptMsg("Valid expression required inside [ ].");
						}
						
						shared_ptr<ASTArraySub> array = make_shared<ASTArraySub>(*ident, expr);
						retVal = make_shared<ASTArrayExpr>(array);
					}
					catch (ParseExcept& e)
					{
						// If this expr is bad, consume until RBracket
						reportError(e);
						consumeUntil(Token::RBracket);
						if (peekToken() == Token::EndOfFile)
						{
							throw EOFExcept();
						}
					}
					
					matchToken(Token::RBracket);
				}
			}
			else if (peekToken() == Token::LParen)
			{
				// Check to make sure this is a function
				if (mCheckSemant && ident->getType() != Type::Function &&
					!ident->isDummy())
				{
					std::string err("'");
					err += ident->getName();
					err += "' is not a function";
					reportSemantError(err);
					consumeUntil(Token::RParen);
					if (peekToken() == Token::EndOfFile)
					{
						throw EOFExcept();
					}
					
					matchToken(Token::RParen);
					
					// Just return our error variable
					retVal = make_shared<ASTIdentExpr>(*mSymbols.getIdentifier("@@variable"));
				}
				else
				{
					consumeToken();
					// A function call can have zero or more arguments
					shared_ptr<ASTFuncExpr> funcCall = make_shared<ASTFuncExpr>(*ident);
					retVal = funcCall;
					
					// Get the number of arguments for this function
					shared_ptr<ASTFunction> func = ident->getFunction();
					
					try
					{
						int currArg = 1;
						int col = mColNumber;
						shared_ptr<ASTExpr> arg = parseExpr();
						while (arg)
						{
							// Check for validity of this argument (for non-dummy functions)
							if (!ident->isDummy())
							{
								// Special case for "printf" since we don't make a node for it
								if (ident->getName() == "printf")
								{
									mNeedPrintf = true;
									if (currArg == 1 && arg->getType() != Type::CharArray)
									{
										reportSemantError("The first parameter to printf must be a char[]");
									}
								}
								else if (mCheckSemant)
								{
									if (currArg > func->getNumArgs())
									{
										std::string err("Function ");
										err += ident->getName();
										err += " takes only ";
										std::ostringstream ss;
										ss << func->getNumArgs();
										err += ss.str();
										err += " arguments";
										reportSemantError(err, col);
									}
									else if (!func->checkArgType(currArg, arg->getType()))
									{
										// If we have an int and the expected arg type is a char,
										// we can do a conversion
										if (arg->getType() == Type::Int &&
											func->getArgType(currArg) == Type::Char)
										{
											arg = intToChar(arg);
										}
										else
										{
											std::string err("Expected expression of type ");
											err += getTypeText(func->getArgType(currArg));
											reportSemantError(err, col);
										}
									}
								}
							}
							
							funcCall->addArg(arg);
							
							currArg++;
							
							if (peekAndConsume(Token::Comma))
							{
								col = mColNumber;
								arg = parseExpr();
								if (!arg)
								{
									throw
									ParseExceptMsg("Comma must be followed by expression in function call");
								}
							}
							else
							{
								break;
							}
						}
					}
					catch (ParseExcept& e)
					{
						reportError(e);
						consumeUntil(Token::RParen);
						if (peekToken() == Token::EndOfFile)
						{
							throw EOFExcept();
						}
					}
					
					// Now make sure we have the correct number of arguments
					if (!ident->isDummy())
					{
						// Special case for printf
						if (ident->getName() == "printf")
						{
							if (funcCall->getNumArgs() == 0)
							{
								reportSemantError("printf requires a minimum of one argument");
							}
						}
						else if (mCheckSemant && funcCall->getNumArgs() < func->getNumArgs())
						{
							std::string err("Function ");
							err += ident->getName();
							err += " requires ";
							std::ostringstream ss;
							ss << func->getNumArgs();
							err += ss.str();
							err += " arguments";
							reportSemantError(err);
						}
					}
					
					matchToken(Token::RParen);
				}
			}
			else
			{
				// Just a plain old ident
				retVal = make_shared<ASTIdentExpr>(*ident);
			}
		}
	}
	
	return retVal;
}

// ++ id
shared_ptr<ASTExpr> Parser::parseIncFactor()
{
	shared_ptr<ASTExpr> retVal;
	Identifier* ident = nullptr;
	
	if (peekAndConsume(Token::Inc)) {
		if (peekToken() == Token::Identifier) {
			ident = getVariable(getTokenTxt());
			consumeToken();
			retVal = make_shared<ASTIncExpr>(*ident);
		}
		
	}
	
	return retVal;
}

// -- id
shared_ptr<ASTExpr> Parser::parseDecFactor()
{
	shared_ptr<ASTExpr> retVal;
	Identifier* ident = nullptr;
	
	if (peekAndConsume(Token::Dec)) {
		if (peekToken() == Token::Identifier) {
			ident = getVariable(getTokenTxt());
			consumeToken();
			retVal = make_shared<ASTDecExpr>(*ident);
		}
		
	}

	return retVal;
}

// & id [ Expr ]
shared_ptr<ASTExpr> Parser::parseAddrOfArrayFactor()
{
	shared_ptr<ASTExpr> retVal;
	shared_ptr<ASTExpr> expr;
	shared_ptr<ASTArraySub> arraySub;
	Identifier* ident = nullptr;
	
	if (peekAndConsume(Token::Addr)) {
		if (peekToken() == Token::Identifier) {
			ident = getVariable(getTokenTxt());
			consumeToken();
			matchToken(Token::LBracket);
			expr = parseExpr();
			if (!expr) {
				throw ParseExceptMsg("Missing required subscript expression.");
			}
			matchToken(Token::RBracket);
			retVal = make_shared<ASTAddrOfArray>(make_shared<ASTArraySub>(*ident, expr));
		} else {
			throw ParseExceptMsg("& must be followed by an identifier.");
		}
	}
	
	return retVal;
}
