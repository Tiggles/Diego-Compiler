#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kittyweed.h"
#include "../parserscanner/kittytree.h"

extern stackT *functionStack;
extern stackT *loopStack;

void weed_error_report ( const char* errorMsg, int lineno ) {
	if (lineno < 0){
		fprintf(stderr, "Error: %s \n",errorMsg);
		exit(WEEDER_FAILURE);
	}
	fprintf(stderr, "Error at line %i: %s \n",lineno,errorMsg);
	exit(WEEDER_FAILURE);
}


BODY *begin_weed ( BODY *body ) {

	fprintf(stderr, "%s\n", "Initializing weeding phase");
	/*
	 * Using a stack to keep track of functions,
	 * need this to attach to statements to make  
	 * sure the return type is correct
	 */

	body = weed_body(body);

	return body;

}

FUNC *weed_function ( FUNC *function ) {

	funcStackPush( functionStack, function );

	if (strcmp(function->head->id, function->tail->id) != 0) {
		weed_error_report("Mismatch in header and tail id of function",
											function->head->lineno);
	}

	weed_head(function->head);
	weed_body(function->body);

	if ( function->body->statement_list->statement->foundReturn == 0 ) {
		weed_error_report("Missing return in function",
						  function->head->lineno);
	}

	funcStackPop( functionStack );

	return function;

}

HEAD *weed_head ( HEAD *header ) {

	header->pdlist = weed_par_decl_list(header->pdlist);
	header->returntype = weed_type(header->returntype);

	return header;

}

TYPE *weed_type ( TYPE *type ) {
	switch ( type->kind ) {
		case TYPE_ARRAY:
			type->value.type = weed_type( type->value.type );
			break;

		case TYPE_RECORD:
			type->value.var_decl_list = 
				weed_var_decl_list( type->value.var_decl_list );
			break;

		default:
			break; //id, int, bool
	}
	return type;

}

PAR_DECL_LIST *weed_par_decl_list ( PAR_DECL_LIST *pdecl ) {
	switch ( pdecl->kind ) {
		case PAR_DECL_LIST_LIST:
			pdecl->var_decl_list = weed_var_decl_list( pdecl->var_decl_list );
			break;

		case PAR_DECL_LIST_EMPTY:
			break;
	}
	return pdecl;

}

VAR_DECL_LIST *weed_var_decl_list ( VAR_DECL_LIST *vdecl ) {
	switch ( vdecl->kind ) {
		case VAR_DECL_LIST_LIST:
			weed_var_decl_list( vdecl->var_decl_list );
			weed_var_type( vdecl->var_type );
			break;

		case VAR_DECL_LIST_TYPE:
			weed_var_type( vdecl->var_type );
			break;

	}
	return vdecl;

}

VAR_TYPE *weed_var_type ( VAR_TYPE *vtype ) {

	weed_type( vtype->type );

	return vtype;
}


OPT_LENGTH *weed_opt_length ( OPT_LENGTH *oplen ) {
	switch ( oplen->kind ) {
		case OPT_LENGTH_EXPRES:
			weed_expression( oplen->exp );
			break;

		case OPT_LENGTH_EMPTY:
			break;
	}
	return oplen;

}


VAR *weed_variable ( VAR *var ) {
	switch ( var->kind ) {
		case VAR_ID:
			break;

		case VAR_ARRAY:
			var->value.var_array.var = 
				weed_variable( var->value.var_array.var );
			var->value.var_array.exp = 
				weed_expression( var->value.var_array.exp );
			break;

		case VAR_RECORD:
			var->value.var_record.var = 
				weed_variable( var->value.var_record.var );
			break;
	}

	return var;

}


ACT_LIST *weed_act_list ( ACT_LIST *actlst ) {
	switch ( actlst->kind ) {
		case ACT_LIST_EXPLIST:
			actlst->exp_list = weed_expression_list( actlst->exp_list );
			break;

		case ACT_LIST_EMPTY:
			break;
	}

	return actlst;

}


EXP_LIST *weed_expression_list ( EXP_LIST *explst ) {
	switch ( explst->kind ) {
		case EXP_LIST_EXP:
			explst->exp = weed_expression(explst->exp);
			break;

		case EXP_LIST_LIST:
			explst->exp = weed_expression(explst->exp);
			explst->explist = weed_expression_list(
				explst->explist);
			break;
	}

	return explst;

}

BODY *weed_body ( BODY *body ){
	body->decl_list = weed_decl_list(body->decl_list);

	body->statement_list = weed_statement_list(body->statement_list);

	return body;

}

DECL_LIST *weed_decl_list ( DECL_LIST *dlst ){
	switch ( dlst->kind ) {
		case DECL_LIST_LIST:
			weed_decl_list(dlst->decl_list);
			weed_declaration(dlst->declaration);
			break;

		case DECL_LIST_EMPTY:
			break;
	}

	return dlst;

}


DECLARATION *weed_declaration ( DECLARATION *decl ){
	switch ( decl->kind ) {
		case DECLARATION_ID:
			decl->value.declaration_id.type =
					weed_type(decl->value.declaration_id.type);
			break;

		case DECLARATION_FUNC:
			decl->value.function = weed_function(decl->value.function);
			break;

		case DECLARATION_VAR:
			decl->value.var_decl_list =
					weed_var_decl_list(decl->value.var_decl_list);
			break;

		default:
			break;
	}
	return decl;

}

STATEMENT_LIST *weed_statement_list ( STATEMENT_LIST *slst ){

	switch ( slst->kind ) {
		case STATEMENT_LIST_LIST:
			slst->statement_list = weed_statement_list(slst->statement_list);
			slst->statement = weed_statement(slst->statement);

			// if-statement with false condition
			// without else-statement removed
			if ( slst->statement_list->statement->kind == STATEMENT_IFBRANCH
				 && slst->statement_list->statement->value.
					statement_if_branch.opt_else->kind == OPT_ELSE_EMPTY ) {

				if ( slst->statement_list->statement->value.
						statement_if_branch.condition->kind == EXPRES_TERM
					 && slst->statement_list->statement->value.
						statement_if_branch.condition->value.
						term->kind == TERM_FALSE) {

					if ( slst->statement_list->statement_list == NULL ) {
						// if if-false-statement is first element make this
						// the first
						slst->kind = STATEMENT_LIST_STATEMENT;
					} else {
						// else jump over if-false statement
						slst->statement_list = slst->statement_list->
								statement_list;
					}
				}
			}

			// remove dead code after return statement
			if ( slst->statement_list->statement->foundReturn ) {
				return slst->statement_list;
			}
			break;

		case STATEMENT_LIST_STATEMENT:
			slst->statement = weed_statement(slst->statement);
			break;
	}
	return slst;

}


STATEMENT *weed_statement ( STATEMENT *st ){

	int isReturnInIfCase;
	int isReturnInElseCase;
	TERM *ifTerm;
	STATEMENT *assignment;
	STATEMENT *increment;

	switch ( st->kind ) {
		case STATEMENT_RETURN:
			st->value.statement_return.exp = 
				weed_expression(st->value.statement_return.exp);

			if ( stackIsEmpty(functionStack) ) {
				weed_error_report("Return not associated with function",
																st->lineno);
			}
			st->foundReturn = 1;
			return st;

		case STATEMENT_IFBRANCH:
			st->value.statement_if_branch.condition =
					weed_expression(st->value.statement_if_branch.condition);

			st->value.statement_if_branch.statement =
					weed_statement(st->value.statement_if_branch.statement);
			
			st->value.statement_if_branch.opt_else =
					weed_opt_else(st->value.statement_if_branch.opt_else);
			

			if ( st->value.statement_if_branch.opt_else->kind
				== OPT_ELSE_EMPTY ) {
				if ( st->value.statement_if_branch.condition->kind
					== EXPRES_TERM ) {
					ifTerm = st->value.statement_if_branch.condition->value.
							term;
					// if true and no else just replace if with it's statement
					if ( ifTerm->kind == TERM_TRUE ) {
						return st->value.statement_if_branch.statement;
					}
				}
			} else {
				isReturnInIfCase = st->value.statement_if_branch.
						statement->foundReturn;
				isReturnInElseCase = st->value.statement_if_branch.opt_else->
						statement->foundReturn;

				// Case where there exists a return in both if and else,
				// can ignore everything after the if/else
				if( isReturnInIfCase && isReturnInElseCase ) {
					st->foundReturn = 1;
				}

				if (st->value.statement_if_branch.condition->kind
					== EXPRES_TERM){
					ifTerm = st->value.statement_if_branch.condition->value.
							term;
					if ( ifTerm->kind == TERM_FALSE ) {
						// if FALSE then we should only look at else
						return st->value.statement_if_branch.opt_else->
								statement;
					}else if ( ifTerm->kind == TERM_TRUE ) {
						// if TRUE then we should only look at if
						return st->value.statement_if_branch.statement;
					}
				}
			}

			break;
		case STATEMENT_LISTS:
			st->value.statement_list =
					weed_statement_list(st->value.statement_list);
			//Look for returns
			st->foundReturn = st->value.statement_list->statement->
					foundReturn;
			break;

		case STATEMENT_ASSIGN:
		case STATEMENT_ADDASSIGN:
		case STATEMENT_SUBASSIGN:
		case STATEMENT_MULASSIGN:
		case STATEMENT_DIVASSIGN:
		case STATEMENT_MODASSIGN:
			st->value.statement_assign.var = 
				weed_variable(st->value.statement_assign.var);
			st->value.statement_assign.exp = 
				weed_expression(st->value.statement_assign.exp);
			break;

		case STATEMENT_WRITE:
			st->value.exp = weed_expression(st->value.exp);
			break;

		case STATEMENT_ALLOCATE:
			st->value.statement_allocate.var = 
				weed_variable(st->value.statement_allocate.var);
			st->value.statement_allocate.opt_length = 
				weed_opt_length(st->value.statement_allocate.opt_length);
			break;

		case STATEMENT_WHILE:
			loopStackPush(loopStack, st);
			st->value.statement_while.condition =
				weed_expression(st->value.statement_while.condition);
			st->value.statement_while.statement = 
				weed_statement(st->value.statement_while.statement);
			loopStackPop(loopStack);
			break;

		case STATEMENT_BREAK:
			if( stackIsEmpty(loopStack) ) {
				weed_error_report("Break outside loop", st->lineno);
			} else {
				st->currentLoop = loopStackPeek(loopStack);
			}
			break;

		case STATEMENT_CONTINUE:
			if( stackIsEmpty(loopStack) ) {
				weed_error_report("Continue outside loop", st->lineno);
			} else {
				st->currentLoop = loopStackPeek(loopStack);
			}
			break;

		case STATEMENT_FOR:
			loopStackPush(loopStack, st);
			assignment = weed_statement(st->value.statement_for.assignment);
			increment = weed_statement(st->value.statement_for.update);
			st->value.statement_for.condition = weed_expression(st->value.
					statement_for.condition);
			st->value.statement_for.statement = weed_statement(st->value.
					statement_for.statement);

			if (!(increment->kind == STATEMENT_ADDASSIGN || increment->kind ==
				STATEMENT_MULASSIGN || increment->kind == STATEMENT_SUBASSIGN
				|| increment->kind == STATEMENT_MULASSIGN || increment->kind
				== STATEMENT_MODASSIGN || increment->kind == STATEMENT_ASSIGN)
				|| !(assignment->kind == STATEMENT_ADDASSIGN ||
                    assignment->kind == STATEMENT_MULASSIGN ||
                    assignment->kind == STATEMENT_SUBASSIGN ||
                    assignment->kind == STATEMENT_MULASSIGN ||
                    assignment->kind == STATEMENT_MODASSIGN ||
                    assignment->kind == STATEMENT_ASSIGN)) {
				weed_error_report("Expected assignment and"
                                          " updating statements to update"
                                          " variable values",
                                  st->lineno);
			}
			loopStackPop(loopStack);
			break;
        case STATEMENT_FOREACH:
            loopStackPush(loopStack, st);
            st->value.statement_foreach.element =
                    weed_variable(st->value.statement_foreach.element);
            st->value.statement_foreach.collection =
                    weed_variable(st->value.statement_foreach.collection);
            st->value.statement_foreach.statement =
                    weed_statement(st->value.statement_foreach.statement);
            loopStackPop(loopStack);
            break;
	}
	return st;

}

OPT_ELSE *weed_opt_else ( OPT_ELSE *opel){
	switch ( opel->kind ) {
		case OPT_ELSE_STATEMENT:
			opel->statement = weed_statement(opel->statement);
			break;

		case OPT_ELSE_EMPTY:
			break;
	}
	return opel;

}

EXPRES *weed_expression( EXPRES *exp ){

	EXPRES *left_exp;
	EXPRES *right_exp;
	TERM *left_term;
	TERM *right_term;
	int temp_res;

	if ( exp->kind == EXPRES_TERM ) {
		exp->value.term = weed_term(exp->value.term);
		return exp;
	}

	exp->value.sides.left = weed_expression(exp->value.sides.left);
	exp->value.sides.right = weed_expression(exp->value.sides.right);

	left_exp = exp->value.sides.left;
	right_exp = exp->value.sides.right;

	switch ( exp->kind ) {

		case EXPRES_AND:
			left_term = left_exp->value.term;
			right_term = right_exp->value.term;

			if((left_term->kind == TERM_TRUE || left_term->kind == TERM_FALSE)
			   && (right_term->kind == TERM_TRUE || right_term->kind
													== TERM_FALSE)){

				if(	left_exp->value.term->kind == TERM_TRUE && 
								right_exp->value.term->kind	== TERM_TRUE){
					exp->value.term = make_TERM_TRUE();
					exp->kind = EXPRES_TERM;
				} else {
					exp->value.term = make_TERM_FALSE();
					exp->kind = EXPRES_TERM;
				}
		 	}

			if ( left_exp->kind == EXPRES_TERM ) {
			/* dominance rule and identity rule for AND with associativity */
				 if ( left_term->kind == TERM_FALSE ) {
					 // dominance rule
					 exp = left_exp;
				 } else if ( left_term->kind == TERM_TRUE ) {
					 // identity rule
					 exp = right_exp;
				 }
			}
			if ( right_exp->kind == EXPRES_TERM ) {
				if ( right_term->kind == TERM_FALSE ) {
					// dominance rule

					if ( left_exp->kind == EXPRES_TERM
						 && left_term->kind == TERM_ACT_LIST ) {
						// because of potential side effects and
						// lazy evaluation we don't skip function calls here
						break;
					}
					exp = right_exp;
				} else if (right_term->kind == TERM_TRUE) {
					// identity rule
					exp = left_exp;
				}
			}
			if ( right_exp->kind == EXPRES_TERM &&
				 left_exp->kind == EXPRES_TERM &&
				 right_term->kind == TERM_VAR &&
				 left_term->kind == TERM_VAR ) {
				if ( right_term->value.var->kind == VAR_ID &&
						left_term->value.var->kind == VAR_ID &&
						strcmp(right_term->value.var->id,
							   left_term->value.var->id) == 0){
					// a && a = a
					free(right_term->value.var);
					free(right_term);
					free(right_exp);

					exp = left_exp;
				}
			}
			break;

		case EXPRES_OR:
			left_term  = left_exp->value.term;
			right_term = right_exp->value.term;

			if( (left_term->kind == TERM_TRUE ||
				left_term->kind == TERM_FALSE) &&
				( right_term->kind == TERM_TRUE ||
						right_term->kind == TERM_FALSE) ) {

				if (left_exp->value.term->kind == TERM_FALSE &&
					right_exp->value.term->kind == TERM_FALSE) {
					exp->value.term = make_TERM_FALSE();
					exp->kind = EXPRES_TERM;
				} else {
					exp->value.term = make_TERM_TRUE();
					exp->kind = EXPRES_TERM;
				}

			}
			// dominance rule and identity rule for OR with associativity
			if ( left_exp->kind == EXPRES_TERM ) {
				if ( left_term->kind == TERM_TRUE ) {
					// dominance rule
					exp = left_exp;
				} else if ( left_term->kind == TERM_FALSE ){
					// identity rule
					exp = right_exp;
				}
			}
			if ( right_exp->kind == EXPRES_TERM ) {
				if (right_term->kind == TERM_TRUE ) {
					// dominance rule
					if ( left_exp->kind == EXPRES_TERM
						 && left_term->kind == TERM_ACT_LIST ) {
						// because of potential side effects and
						// lazy evaluation we don't skip function calls here
						break;
					}
					exp = right_exp;
				} else if (right_term->kind == TERM_FALSE ){
					// identity rule
					exp = left_exp;
				}
			}
			if ( right_exp->kind == EXPRES_TERM &&
				 left_exp->kind == EXPRES_TERM &&
				 right_term->kind == TERM_VAR &&
				 left_term->kind == TERM_VAR ){
				if (right_term->value.var->kind == VAR_ID &&
					left_term->value.var->kind == VAR_ID &&
					strcmp(right_term->value.var->id,
						   left_term->value.var->id) == 0){
					// a || a = a
					free(right_term->value.var);
					free(right_term);
					free(right_exp);

					exp = left_exp;
				}
			}
			break;

		case EXPRES_PLUS:
			if( left_exp->kind == EXPRES_TERM &&
					right_exp->kind == EXPRES_TERM ){
				left_term = left_exp->value.term;
				right_term = right_exp->value.term;
				if(left_term->kind != TERM_NUM || 
				   right_term->kind != TERM_NUM ) {
					break;
				}

				exp->value.term = make_TERM_NUM(left_term->value.intconst +
												right_term->value.intconst);

				exp->kind = EXPRES_TERM;
			}
			break;

		case EXPRES_MINUS:
			if( left_exp->kind == EXPRES_TERM &&
					right_exp->kind == EXPRES_TERM ){

				left_term = left_exp->value.term;
				right_term = right_exp->value.term;

				if(left_term->kind != TERM_NUM || 
				   right_term->kind != TERM_NUM ) {
					break;
				}

				exp->value.term = make_TERM_NUM(left_term->value.intconst - 
												right_term->value.intconst);

				exp->kind = EXPRES_TERM;
			}
			break;

		case EXPRES_TIMES:
			if( left_exp->kind == EXPRES_TERM &&
					right_exp->kind == EXPRES_TERM ){

				left_term = left_exp->value.term;
				right_term = right_exp->value.term;

				if(left_term->kind != TERM_NUM || 
				   right_term->kind != TERM_NUM ) {
					break;
				}

				exp->value.term = make_TERM_NUM(left_term->value.intconst * 
												right_term->value.intconst);

				exp->kind = EXPRES_TERM;

			}
			break;

		case EXPRES_DIVIDE:
		case EXPRES_MODULO:
			if( left_exp->kind == EXPRES_TERM &&
				right_exp->kind == EXPRES_TERM ){

				left_term = left_exp->value.term;
				right_term = right_exp->value.term;

				if(left_term->kind != TERM_NUM ||
				   right_term->kind != TERM_NUM ) {
					break;
				}

				if(right_term->value.intconst == 0){
					weed_error_report("Division by 0", exp->lineno);
				}

				if ( exp->kind == EXPRES_DIVIDE ) {
					temp_res = left_term->value.intconst /
							right_term->value.intconst;
				} else {
					temp_res = left_term->value.intconst %
							right_term->value.intconst;
				}

				exp->value.term = make_TERM_NUM(temp_res);
				exp->kind = EXPRES_TERM;
			}
			break;

		case EXPRES_LEQ:
			if( left_exp->kind == EXPRES_TERM &&
					right_exp->kind == EXPRES_TERM ){

				left_term = left_exp->value.term;
				right_term = right_exp->value.term;

				if(left_term->kind == TERM_NUM &&
				   	right_term->kind == TERM_NUM ) {
					if( left_term->value.intconst <=
							right_term->value.intconst ){
						exp->value.term = make_TERM_TRUE();
						exp->kind = EXPRES_TERM;
					} else {
						exp->value.term = make_TERM_FALSE();
						exp->kind = EXPRES_TERM;
					}
				}

				if ( right_term->kind == TERM_VAR &&
					 left_term->kind == TERM_VAR ){
					if (right_term->value.var->kind == VAR_ID &&
						left_term->value.var->kind == VAR_ID &&
						strcmp(right_term->value.var->id,
							   left_term->value.var->id) == 0){
						// a == a -> true
						free(left_term->value.var);
						free(right_term->value.var);
						free(left_term);
						free(right_term);
						free(left_exp);
						free(right_exp);

						exp = make_EXPRES_TERM(make_TERM_TRUE());
					}
				}
			}
			break;

		case EXPRES_GEQ:
			if( left_exp->kind == EXPRES_TERM &&
					right_exp->kind == EXPRES_TERM ){

				left_term = left_exp->value.term;
				right_term = right_exp->value.term;

				if( left_term->kind == TERM_NUM &&
				   right_term->kind == TERM_NUM ) {

					if( left_term->value.intconst >=
							right_term->value.intconst ){
						exp->value.term = make_TERM_TRUE();
						exp->kind = EXPRES_TERM;
					} else {
						exp->value.term = make_TERM_FALSE();
						exp->kind = EXPRES_TERM;
					}
				}

				if ( right_term->kind == TERM_VAR &&
					 left_term->kind == TERM_VAR ){
					if (right_term->value.var->kind == VAR_ID &&
						left_term->value.var->kind == VAR_ID &&
						strcmp(right_term->value.var->id,
							   left_term->value.var->id) == 0){
						// a == a -> true
						free(left_term->value.var);
						free(right_term->value.var);
						free(left_term);
						free(right_term);
						free(left_exp);
						free(right_exp);

						exp = make_EXPRES_TERM(make_TERM_TRUE());
					}
				}

			}
			break;

		case EXPRES_EQ:
			if( left_exp->kind == EXPRES_TERM &&
					right_exp->kind == EXPRES_TERM ){

				left_term = left_exp->value.term;
				right_term = right_exp->value.term;

				if(left_term->kind == TERM_NUM &&
				    right_term->kind == TERM_NUM ) { 
					if( left_term->value.intconst ==
							right_term->value.intconst ){
						exp->value.term = make_TERM_TRUE();
						exp->kind = EXPRES_TERM;
					} else {
						exp->value.term = make_TERM_FALSE();
						exp->kind = EXPRES_TERM;
					}
				}

				if ( right_term->kind == TERM_VAR &&
					 left_term->kind == TERM_VAR ){
					if (right_term->value.var->kind == VAR_ID &&
						left_term->value.var->kind == VAR_ID &&
						strcmp(right_term->value.var->id,
							   left_term->value.var->id) == 0){
						// a == a -> true
						free(left_term->value.var);
						free(right_term->value.var);
						free(left_term);
						free(right_term);
						free(left_exp);
						free(right_exp);

						exp = make_EXPRES_TERM(make_TERM_TRUE());
					}
				}
			}
			break;

		case EXPRES_NEQ:
			if( left_exp->kind == EXPRES_TERM &&
					right_exp->kind == EXPRES_TERM ){

				left_term = left_exp->value.term;
				right_term = right_exp->value.term;

				if(left_term->kind == TERM_NUM && 
				    right_term->kind == TERM_NUM ) { 
					if( left_term->value.intconst != 
						right_term->value.intconst ){
						exp->value.term = make_TERM_TRUE();
						exp->kind = EXPRES_TERM;
					} else {
						exp->value.term = make_TERM_FALSE();
						exp->kind = EXPRES_TERM;
					}
				}

				if ( right_term->kind == TERM_VAR &&
					 left_term->kind == TERM_VAR ){
					if (right_term->value.var->kind == VAR_ID &&
						left_term->value.var->kind == VAR_ID &&
						strcmp(right_term->value.var->id,
							   left_term->value.var->id) == 0){
						// a != a -> FALSE
						free(left_term->value.var);
						free(right_term->value.var);
						free(left_term);
						free(right_term);
						free(left_exp);
						free(right_exp);

						exp = make_EXPRES_TERM(make_TERM_FALSE());
					}
				}
			}
			break;

		case EXPRES_GREATER:
			if( left_exp->kind == EXPRES_TERM &&
					right_exp->kind == EXPRES_TERM ){

				left_term = left_exp->value.term;
				right_term = right_exp->value.term;

				if(left_term->kind == TERM_NUM &&
				   right_term->kind == TERM_NUM ) {
					if( left_term->value.intconst >
						right_term->value.intconst ){
						exp->value.term = make_TERM_TRUE();
						exp->kind = EXPRES_TERM;
					} else {
						exp->value.term = make_TERM_FALSE();
						exp->kind = EXPRES_TERM;
					}
				}

				if ( right_term->kind == TERM_VAR &&
					 left_term->kind == TERM_VAR ){
					if (right_term->value.var->kind == VAR_ID &&
						left_term->value.var->kind == VAR_ID &&
						strcmp(right_term->value.var->id,
							   left_term->value.var->id) == 0){
						// a > a -> FALSE
						free(left_term->value.var);
						free(right_term->value.var);
						free(left_term);
						free(right_term);
						free(left_exp);
						free(right_exp);

						exp = make_EXPRES_TERM(make_TERM_FALSE());
					}
				}
			}
			break;

		case EXPRES_LESS:
			if( left_exp->kind == EXPRES_TERM &&
					right_exp->kind == EXPRES_TERM ){

				left_term = left_exp->value.term;
				right_term = right_exp->value.term;

				if( left_term->kind == TERM_NUM &&
				    right_term->kind == TERM_NUM ) {
					if( left_term->value.intconst <
							right_term->value.intconst ){
						exp->value.term = make_TERM_TRUE();
						exp->kind = EXPRES_TERM;
					} else {
						exp->value.term = make_TERM_FALSE();
						exp->kind = EXPRES_TERM;
					}
				}

				if ( right_term->kind == TERM_VAR &&
					 left_term->kind == TERM_VAR ){
					if (right_term->value.var->kind == VAR_ID &&
						left_term->value.var->kind == VAR_ID &&
						strcmp(right_term->value.var->id,
							   left_term->value.var->id) == 0){
						// a < a -> FALSE
						free(left_term->value.var);
						free(right_term->value.var);
						free(left_term);
						free(right_term);
						free(left_exp);
						free(right_exp);

						exp = make_EXPRES_TERM(make_TERM_FALSE());
					}
				}
			}
			break;
		default:
			break;
	}

	return exp;

}


TERM *weed_term ( TERM *term) {

	TERM *tempTerm;

	switch ( term->kind ) {

		case TERM_VAR:
			term->value.var = weed_variable(term->value.var);
			break;

		case TERM_ACT_LIST:
			term->value.term_act_list.actlist =
					weed_act_list(term->value.term_act_list.actlist);
			break;

		case TERM_NOT:
			term->value.term = weed_term(term->value.term);

			if (term->value.term->kind == TERM_NOT){
				// double negative == positive
				tempTerm = term->value.term;
				term = term->value.term->value.term;
				free(tempTerm);
			}
			if (term->value.term->kind == TERM_TRUE){
				term->kind = TERM_FALSE;
			}
			if (term->value.term->kind == TERM_FALSE){
				term->kind = TERM_TRUE;
			}
			break;

		case TERM_UMINUS:
			term->value.term = weed_term(term->value.term);

			if ( term->value.term->kind == TERM_UMINUS ) {
				// like in NOT -> discard double negative
				tempTerm = term->value.term;
				term = term->value.term->value.term;
				free(tempTerm);
			}
			if ( term->value.term->kind == TERM_NUM ) {
				tempTerm = term->value.term;
				term->value.term = NULL;
				free(term);
				term = tempTerm;
				term->value.intconst = -term->value.intconst;
			}
			break;

		case TERM_PARENTESES:
			term->value.exp = weed_expression(term->value.exp);

			if ( term->value.exp->kind == EXPRES_TERM ) {
				// if (23), (true) or (false) -> discard parentheses
				tempTerm = term->value.exp->value.term;
				if ( tempTerm->kind == TERM_NUM
					 || tempTerm->kind == TERM_TRUE
					 || tempTerm->kind == TERM_FALSE ) {
					term->value.exp = NULL;
					free(term->value.exp);
					free(term);
					term = tempTerm;
				}
			}
			break;

		case TERM_ABS:
			term->value.exp = weed_expression(term->value.exp);
			if (term->value.exp->kind == EXPRES_TERM) {
				tempTerm = term->value.exp->value.term;

				if (tempTerm->kind == TERM_NUM) {
					if (tempTerm->value.intconst < 0) {
						// negate and remove abs operation
						tempTerm->value.intconst *= (-1);

						term->value.exp->value.term = NULL;
						free(term);
						free(term->value.exp);

						term = tempTerm;
					} else {
						// positive, remove abs operation
						term->value.exp->value.term = NULL;
						free(term);
						free(term->value.exp);

						term = tempTerm;
					}
				}
			}
			break;

		default:
			break;
	}
return term;
}




