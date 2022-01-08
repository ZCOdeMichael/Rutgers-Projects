package app;

import java.io.*;
import java.util.*;
import java.util.regex.*;

import structures.Stack;

public class Expression {

	public static String delims = " \t*+-/()[]";
			
    /**
     * Populates the vars list with simple variables, and arrays lists with arrays
     * in the expression. For every variable (simple or array), a SINGLE instance is created 
     * and stored, even if it appears more than once in the expression.
     * At this time, values for all variables and all array items are set to
     * zero - they will be loaded from a file in the loadVariableValues method.
     * 
     * @param expr The expression
     * @param vars The variables array list - already created by the caller
     * @param arrays The arrays array list - already created by the caller
     */
    public static void 
    makeVariableLists(String expr, ArrayList<Variable> vars, ArrayList<Array> arrays) {
    	/** COMPLETE THIS METHOD **/
    	/** DO NOT create new vars and arrays - they are already created before being sent in
    	 ** to this method - you just need to fill them in.
    	 **/
    	
    	String newExpr = removeSpaces(expr);

    	for(int i = 0; i <= newExpr.length()-1; i++) {
    		
    		if(!isOP(newExpr.charAt(i)) && !isNumber(newExpr.charAt(i))){
    		
    			if(i == newExpr.length()-1) {
    				vars.add(new Variable(newExpr.substring(i)));
    			}else {
    				if(newExpr.indexOf('[', i) != -1 && nextOP(i, newExpr) != -1) {
	    				if(nextOP(i, newExpr) < newExpr.indexOf('[', i)) {
	    					vars.add(new Variable(newExpr.substring(i, nextOP(i, newExpr))));
	    					i = nextOP(i, newExpr);
		    				
		    			}else{
		    				arrays.add(new Array(newExpr.substring(i, newExpr.indexOf('[', i))));
		    				i = newExpr.indexOf('[', i);
		    			}
    				}else if(newExpr.indexOf('[', i) == -1 && nextOP(i, newExpr) != -1){
    					vars.add(new Variable(newExpr.substring(i, nextOP(i, newExpr))));
    					i = nextOP(i, newExpr);
		    			
    				}else if(newExpr.indexOf('[', i) != -1 && nextOP(i, newExpr) == -1){
    					arrays.add(new Array(newExpr.substring(i, newExpr.indexOf('[', i))));
    					i = newExpr.indexOf('[', i);
		    			
    				}else {
    					vars.add(new Variable(newExpr.substring(i)));
    					break;
    				}
        				
    			}
    		}
    	}
    		
    }

    private static int nextOP(int start, String expr) {
    	int smallest = -1;
    	
    	if(expr.indexOf('*', start) != -1) {
    		smallest = expr.indexOf('*', start);
    	}else if(expr.indexOf('-', start) != -1) {
    		smallest = expr.indexOf('-', start);
    	}else if(expr.indexOf('/', start) != -1) {
    		smallest = expr.indexOf('/', start);
    	}else if(expr.indexOf('+', start) != -1){
    		smallest = expr.indexOf('+', start);
    	}else if(expr.indexOf(')', start) != -1){
    		smallest = expr.indexOf(')', start);
    	}else {
    		smallest = expr.indexOf(']', start);
    	}
    	
    	if(smallest != -1) {
    		if(smallest > expr.indexOf('*', start) && expr.indexOf('*', start) != -1){
    			smallest = expr.indexOf('*', start);
    		}
    		if(smallest > expr.indexOf('-', start) && expr.indexOf('-', start) != -1){
    			smallest = expr.indexOf('-', start);
    		}
    		if(smallest > expr.indexOf('/', start) && expr.indexOf('/', start) != -1){
    			smallest = expr.indexOf('/', start);
    		}
    		if(smallest > expr.indexOf('+', start) && expr.indexOf('+', start) != -1){
    			smallest = expr.indexOf('+', start);
    		}
    		if(smallest > expr.indexOf(')', start) && expr.indexOf(')', start) != -1){
    			smallest = expr.indexOf(')', start);
    		}
    		if(smallest > expr.indexOf(']', start) && expr.indexOf(']', start) != -1){
    			smallest = expr.indexOf(']', start);
    		}
    		
    		
    	}
    	return smallest;
    }
    
    
    
    private static boolean isOP(char c) {
    	return (c == '+' || c == '-' || c == '/' || c == '*' || c == '(' || c == ')' || c == ']');
    }
    
    private static boolean isNumber(char c) {
    	return (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9'); 
    }
    //Probably useless (NOT)
    private static String removeSpaces(String expr) {
    	if(expr.equals("")) {
    		return expr;
    	}
    	
    	for(int i = 0; i < expr.length(); i++) {
    		if(expr.charAt(i) == ' ') {
    			expr = expr.substring(0, i) + expr.substring(i+1);
    		}
    	}
    	return expr;
    }
    
    /**
     * Loads values for variables and arrays in the expression
     * 
     * @param sc Scanner for values input
     * @throws IOException If there is a problem with the input 
     * @param vars The variables array list, previously populated by makeVariableLists
     * @param arrays The arrays array list - previously populated by makeVariableLists
     */
    public static void 
    loadVariableValues(Scanner sc, ArrayList<Variable> vars, ArrayList<Array> arrays) 
    throws IOException {
        while (sc.hasNextLine()) {
            StringTokenizer st = new StringTokenizer(sc.nextLine().trim());
            int numTokens = st.countTokens();
            String tok = st.nextToken();
            Variable var = new Variable(tok);
            Array arr = new Array(tok);
            int vari = vars.indexOf(var);
            int arri = arrays.indexOf(arr);
            if (vari == -1 && arri == -1) {
            	continue;
            }
            int num = Integer.parseInt(st.nextToken());
            if (numTokens == 2) { // scalar symbol
                vars.get(vari).value = num;
            } else { // array symbol
            	arr = arrays.get(arri);
            	arr.values = new int[num];
                // following are (index,val) pairs
                while (st.hasMoreTokens()) {
                    tok = st.nextToken();
                    StringTokenizer stt = new StringTokenizer(tok," (,)");
                    int index = Integer.parseInt(stt.nextToken());
                    int val = Integer.parseInt(stt.nextToken());
                    arr.values[index] = val;              
                }
            }
        }
    }
    private static int nextOP1(int start, String expr) {
    	
    	int smallest = -1;
    	
    	for(int i = start; i < expr.length(); i++) {
    		if(expr.charAt(i) == '[') {
    			i = entireArray(i, expr);
    		}
    		if(expr.charAt(i) == '(') {
    			i = entirePara(i, expr);
    		}
    		if(expr.charAt(i) == '+' || expr.charAt(i) == '-' || expr.charAt(i) == '*' || expr.charAt(i) == '/' || expr.charAt(i) == '+' ) {
    			smallest = i;
    			break;
    		}
    	}
    	return smallest;
    }
    
    /**
     * Evaluates the expression.
     * 
     * @param vars The variables array list, with values for all variables in the expression
     * @param arrays The arrays array list, with values for all array items
     * @return Result of evaluation
     */
    public static float 
    evaluate(String expr, ArrayList<Variable> vars, ArrayList<Array> arrays) {
    	expr = removeSpaces(expr);
    	
    	if(expr.length() == 0) {
    		return 0;
    	}
    	//System.out.println(expr); // Prints expr
    	if(nextOP(0, expr) == -1 && !isNumber(expr.charAt(0))) {
    		return getVars(expr, vars);
    	}else if(nextOP(0, expr) == -1 && isNumber(expr.charAt(0))) {
    		return Float.parseFloat(expr);
    	}else if(!isNumber(expr.charAt(0)) && onlyArray(0, expr)){	
    		return getArray(expr.substring(0, expr.indexOf('[', 0)),arrays)[(int)evaluate(expr.substring(expr.indexOf('[',0)+1, entireArray(expr.indexOf('[', 0),expr)), vars, arrays)];
    	}else{
    		if(expr.charAt(0) == '-' && expr.charAt(0) != '(') {
    			if(nextOP1(1, expr) != -1) {
    				//System.out.println("Hello2");
    				//System.out.println(nextCharOP(1, expr));
    				if(nextCharOP(1, expr) == '-' || nextCharOP(1, expr) == '+') {
    					//System.out.println("Hello3");
    					if(nextCharOP(1, expr) == '-' ) {
	    					return -1*evaluate(expr.substring(1, nextOP1(1, expr)), vars, arrays) + evaluate(expr.substring(nextOP1(1, expr)), vars, arrays);
	    				}else {
	    					return -1*evaluate(expr.substring(1, nextOP1(1, expr)), vars, arrays) + evaluate(expr.substring(nextOP1(1, expr)+1), vars, arrays);
	    				}
	    			}else if(nextCharOP(1, expr) == '*' || nextCharOP(1, expr) == '/') {//implement
	    				//System.out.println("Hello1");
	    				if(lastMulti(1, expr) != -1) {
	    					if(expr.charAt(lastMulti(1, expr)) == '+') {
	    						//System.out.println("Hello");
	    						return -1*evaluate(expr.substring(1, lastMulti(1, expr)), vars, arrays) + evaluate(expr.substring(lastMulti(1, expr)+1), vars, arrays);
	    					}else {
	    						return -1*evaluate(expr.substring(1, lastMulti(1, expr)), vars, arrays) + evaluate(expr.substring(lastMulti(1, expr)), vars, arrays);
	    					}
	    				}else {
	    					return -1*evaluate(expr.substring(1), vars, arrays);
	    				}
	    			}else {
	    				//System.out.println("Hello 4");
	    				return -1*evaluate(expr.substring(1), vars, arrays);
	    			}
    			}else {
    				return -1*evaluate(expr.substring(1), vars, arrays);
    			}
    		}else if(expr.charAt(0) != '('){//+-*/
    			if(nextCharOP(0, expr) == '+') {//+
    				return evaluate(expr.substring(0, nextOP1(0, expr)), vars, arrays) + evaluate(expr.substring(nextOP1(0, expr)+1), vars, arrays);
    			}else if(nextCharOP(0, expr) == '-') {//-
    				return evaluate(expr.substring(0, nextOP1(0, expr)), vars, arrays) + evaluate(expr.substring(nextOP1(0,expr)), vars, arrays);
    			}else if(nextCharOP(0, expr) == '*') {//*
    				if(lastMulti(1, expr) != -1) {
    					if(expr.charAt(lastMulti(1, expr)) == '+') {
    						return (evaluate(expr.substring(0, nextOP1(0, expr)), vars, arrays) * evaluate(expr.substring(nextOP1(0, expr)+1, lastMulti(0, expr)), vars, arrays)) + evaluate(expr.substring(lastMulti(1, expr)+1), vars, arrays);
    					}else {
    						return (evaluate(expr.substring(0, nextOP1(0, expr)), vars, arrays) * evaluate(expr.substring(nextOP1(0, expr)+1, lastMulti(0, expr)), vars, arrays)) + evaluate(expr.substring(lastMulti(1, expr)), vars, arrays);
    					}
    				}else {
    					return (evaluate(expr.substring(0, nextOP1(0, expr)), vars, arrays) * evaluate(expr.substring(nextOP1(0, expr)+1), vars, arrays));
    				}
    			}else {// / NOT RIGHT FIX
    				if(lastMulti(0, expr) != -1) {// * and / not entire string
    					
    					if(expr.charAt(lastMulti(0, expr)) == '+') {
    						
    						return (evaluate(expr.substring(0, lastMulti(0,expr)), vars, arrays)) + evaluate(expr.substring(lastMulti(0, expr)+1), vars, arrays);
    					}else {
    						
    						return (evaluate(expr.substring(0, lastMulti(0,expr)), vars, arrays)) + evaluate(expr.substring(lastMulti(0, expr)), vars, arrays);
    					}
    				}else {// * and / is entire string
    					if(expr.charAt(nextOP(0, expr)) == '/') { // division
    						
    						if(nextOP(nextOP(0,expr)+1, expr) == -1) {//only two terms in expr
    							
    							return ((evaluate(expr.substring(0, nextOP(0, expr)), vars, arrays)) / (evaluate(expr.substring(nextOP(0, expr)+1), vars, arrays)));
    	    					
    						}else {//more than two terms
    							if(expr.charAt(nextOP(nextOP(0,expr)+1, expr)) == '/') {
    								
    								return ((evaluate(expr.substring(0, nextOP(0, expr)), vars, arrays)) / (evaluate(expr.substring(nextOP(0, expr)+1, nextOP(nextOP(0,expr)+1, expr)), vars, arrays)))
    										* (evaluate("1/"+expr.substring(nextOP(nextOP(0,expr)+1, expr)+1), vars, arrays));
    							}else {
    								
    								return ((evaluate(expr.substring(0, nextOP(0, expr)), vars, arrays)) / (evaluate(expr.substring(nextOP(0, expr)+1, nextOP(nextOP(0,expr)+1, expr)), vars, arrays)))
    										* (evaluate(expr.substring(nextOP(nextOP(0,expr)+1, expr)+1), vars, arrays));
    							}
    						}
    					}else {// multiplication
    						
    						return evaluate(expr.substring(0, nextOP(0,expr)), vars, arrays) * evaluate(expr.substring(nextOP(0,expr)+1), vars, arrays);
    					}
    					
    				}
    			}
    		}else {//() // array can be before this maybe one of sub base case if that is the case fix isParaArr
    			if(entirePara(0, expr)+1 != expr.length()) {
    			
    				if(expr.charAt(entirePara(0, expr)+1) == '+') {// + after para
	    				return evaluate(expr.substring(1,entirePara(0, expr)), vars, arrays) + evaluate(expr.substring(entirePara(0, expr)+2), vars, arrays);
	    			}else if(expr.charAt(entirePara(0, expr)+1) == '-') {// - after para
	    				return evaluate(expr.substring(1,entirePara(0, expr)), vars, arrays) + evaluate(expr.substring(entirePara(0, expr)+1), vars, arrays);
	    			}else if(expr.charAt(entirePara(0, expr)+1) == '*') {// * after para
	    				if(lastMulti(0, expr) != -1) {//not entire string
	    					if(expr.charAt(lastMulti(0, expr)) == '+') {
		    					return (evaluate(expr.substring(1, entirePara(0, expr)), vars, arrays) * evaluate(expr.substring(entirePara(0,expr)+2, lastMulti(0, expr)), vars, arrays))
		    							+ evaluate(expr.substring(lastMulti(0,expr)+1), vars, arrays);
		    				}else {
		    					return (evaluate(expr.substring(1, entirePara(0, expr)), vars, arrays) * evaluate(expr.substring(entirePara(0,expr)+2, lastMulti(0, expr)), vars, arrays))
		    							+ evaluate(expr.substring(lastMulti(0,expr)), vars, arrays);
		    				}
	    				}else {//entire string
	    					return (evaluate(expr.substring(1, entirePara(0,expr)), vars, arrays) * evaluate(expr.substring(entirePara(0, expr)+2), vars, arrays));
	    				}
	    			}else if(expr.charAt(entirePara(0, expr)+1) == '/') {// / after para
	    				if(lastMulti(0, expr) != -1) {//not entire string
	    					if(expr.charAt(lastMulti(0, expr)) == '+') {
		    					return (evaluate(expr.substring(1, entirePara(0, expr)), vars, arrays) * evaluate("1/" + expr.substring(entirePara(0,expr)+2, lastMulti(0, expr)), vars, arrays))
		    							+ evaluate(expr.substring(lastMulti(0,expr)+1), vars, arrays);
		    				}else {
		    					return (evaluate(expr.substring(1, entirePara(0, expr)), vars, arrays) * evaluate("1/" + expr.substring(entirePara(0,expr)+2, lastMulti(0, expr)), vars, arrays))
		    							+ evaluate(expr.substring(lastMulti(0,expr)), vars, arrays);
		    				}
	    				}else {//entire string
	    					return (evaluate(expr.substring(1, entirePara(0,expr)), vars, arrays) * evaluate("1/" + expr.substring(entirePara(0, expr)+2), vars, arrays));
	    				}
	    			}
    			}else {//nothing after para
    				return evaluate(expr.substring(1, entirePara(0, expr)), vars, arrays);
    			}
    		}
    	}
    	
    	
    	
    	
    	
    	
    	
    	
    
    	
    	return 0;
    }
    private static int nextPluMin(int start, String expr) {
    	int result = -1;
    	for(int i = start; i < expr.length(); i++) {
    		if(expr.charAt(i) == '+' || expr.charAt(i) == '-') {
    			
    		}
    	}
    	return result;
    }
    
    private static boolean onlyArray(int start, String expr) {
    	boolean result = true;
    	if(isArray(start, expr) && expr.indexOf('[', 0) != -1) {
    		//System.out.println(expr.indexOf('[', 0));
    		//System.out.println(expr);
    		for(int i = entireArray(expr.indexOf('[', 0), expr); i < expr.length(); i++) {
    			if(expr.charAt(i) == '+' || expr.charAt(i) == '-' || expr.charAt(i) == '*' || expr.charAt(i) == '/' || expr.charAt(i) == '(' || expr.charAt(i) == ')') {
        			result = false;
    				break;
        		}
        	}
    	}else {
    		result = false;
    	}
    	return result;
    	
    }
    
    private static boolean isArray(int start, String expr) {
    	boolean result = false;
    	for(int i = 0; i < expr.length(); i++) {
    		if(expr.charAt(i) == '+' || expr.charAt(i) == '-' || expr.charAt(i) == '*' || expr.charAt(i) == '/' || expr.charAt(i) == '(' || expr.charAt(i) == ')') {
    			break;
    		}
    		if(expr.charAt(i) == '[') {
    			result = true;
    			break;
    		}
    	}
    	return result;
    }
    
    private static int lastMult(int start, String expr) {
    	for(int i = start; i < expr.length(); i++) {
    		if(expr.charAt(i) == '(' || expr.charAt(i) == '[') {
				if(expr.charAt(i) == '[')
					i = entireArray(i, expr);
				else
					i = entirePara(i, expr);
			}
    		if(expr.charAt(i) == '+' || expr.charAt(i) == '-' || expr.charAt(i) == '/') {
    			start = i;
    			break;
    		}
    	}
    	return start;
    }
    private static int lastDiv(int start, String expr) {
    	for(int i = start; i < expr.length(); i++) {
    		if(expr.charAt(i) == '(' || expr.charAt(i) == '[') {
				if(expr.charAt(i) == '[')
					i = entireArray(i, expr);
				else
					i = entirePara(i, expr);
			}
    		if(expr.charAt(i) == '+' || expr.charAt(i) == '-' || expr.charAt(i) == '*') {
    			start = i;
    			break;
    		}
    		if(i+1 == expr.length()) {
    			return expr.length();
    		}
    	}
    	return start;
    }
    
    private static char nextCharOP(int start, String expr) {
    	return expr.charAt(nextOP1(start, expr));
    }
    
    private static float getVars(String var, ArrayList<Variable> vars) {
    	for(int i = 0; i < vars.size(); i++) {
    		if(vars.get(i).name.equals(var)) {
    			return vars.get(i).value;
    		}
    	}
    	return -1;
    }
    private static int[] getArray(String array, ArrayList<Array> arrays) {
    	for(int i = 0; i < arrays.size(); i++) {
    		if(arrays.get(i).name.equals(array)) {
    			return arrays.get(i).values;
    		}
    	}
    	return null;
    }
    
	private static int lastMulti(int index, String expr) {
		int result = -1;
		for(int i = index; i < expr.length(); i++) {
			if(expr.charAt(i) == '(' || expr.charAt(i) == '[') {
				if(expr.charAt(i) == '[')
					i = entireArray(i, expr);
				else
					i = entirePara(i, expr);
			}
			if(expr.charAt(i) == '+' || expr.charAt(i) == '-' || i+1 == expr.length()){
				if(i+1 == expr.length()) {
					result = -1;
					break;
				}else {
					result = i;
					break;
				}
			}
		}
		return result;
	}
	
    private static int entirePara(int index, String expr) {
    	Stack<Integer> temp = new Stack<>();
    	do {
    		if(expr.charAt(index) == '(') {
    			temp.push(1);
    		}
    		if(expr.charAt(index) == ')') {
    			temp.pop();
    		}
    		index++;
    	}while(!temp.isEmpty());
    	return index-1;
    }
    
    private static int entireArray(int index, String expr) {
    	Stack<Integer> temp = new Stack<>();
    	do {
    		if(expr.charAt(index) == '[') {
    			temp.push(1);
    		}
    		if(expr.charAt(index) == ']') {
    			temp.pop();
    		}
    		index++;
    	}while(!temp.isEmpty());
    	return index-1;
    }
}