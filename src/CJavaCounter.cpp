//! Code counter class methods for the Java language.
/*!
* \file CJavaCounter.cpp
*
* This file contains the code counter class methods for the Java language.
*/

#include "CJavaCounter.h"

/*!
* Constructs a CJavaCounter object.
*/
CJavaCounter::CJavaCounter()
{
	classtype = JAVA;
	language_name = "Java";

	file_extension.push_back(".java");

	directive.push_back("import");
	directive.push_back("package");

	data_name_list.push_back("abstract");
	data_name_list.push_back("ArrayList");
	data_name_list.push_back("boolean");
	data_name_list.push_back("byte");
	data_name_list.push_back("char");
	data_name_list.push_back("class");
	data_name_list.push_back("double");
	data_name_list.push_back("extends");
	data_name_list.push_back("float");
	data_name_list.push_back("HashMap");
	data_name_list.push_back("HashSet");
	data_name_list.push_back("implements");
	data_name_list.push_back("int");
	data_name_list.push_back("interface");
	data_name_list.push_back("LinkedHashMap");
	data_name_list.push_back("LinkedList");
	data_name_list.push_back("long");
	data_name_list.push_back("native");
	data_name_list.push_back("private");
	data_name_list.push_back("protected");
	data_name_list.push_back("public");
	data_name_list.push_back("short");
	data_name_list.push_back("static");
	data_name_list.push_back("String");
	data_name_list.push_back("TreeMap");
	data_name_list.push_back("Vector");
	data_name_list.push_back("void");
	data_name_list.push_back("volatile");

	exec_name_list.push_back("break");
	exec_name_list.push_back("case");
	exec_name_list.push_back("catch");
	exec_name_list.push_back("continue");
	exec_name_list.push_back("default");
	exec_name_list.push_back("do");
	exec_name_list.push_back("else");
	exec_name_list.push_back("finally");
	exec_name_list.push_back("for");
	exec_name_list.push_back("if");	
	exec_name_list.push_back("new");
	exec_name_list.push_back("return");
	exec_name_list.push_back("super");
	exec_name_list.push_back("switch");
	exec_name_list.push_back("this");
	exec_name_list.push_back("throw");
	exec_name_list.push_back("throws");
	exec_name_list.push_back("try");
	exec_name_list.push_back("while");

	math_func_list.push_back("Math.abs");
	math_func_list.push_back("Math.cbrt");
	math_func_list.push_back("Math.ceil");
	math_func_list.push_back("Math.copySign");
	math_func_list.push_back("Math.E");
	math_func_list.push_back("Math.exp");
	math_func_list.push_back("Math.expm1");
	math_func_list.push_back("Math.floor");
	math_func_list.push_back("Math.getExponent");
	math_func_list.push_back("Math.hypot");
	math_func_list.push_back("Math.IEEEremainder");	
	math_func_list.push_back("Math.max");
	math_func_list.push_back("Math.min");
	math_func_list.push_back("Math.nextAfter");
	math_func_list.push_back("Math.nextUp");
	math_func_list.push_back("Math.PI");
	math_func_list.push_back("Math.pow");
	math_func_list.push_back("Math.random");
	math_func_list.push_back("Math.rint");
	math_func_list.push_back("Math.round");
	math_func_list.push_back("Math.scalb");
	math_func_list.push_back("Math.signum");
	math_func_list.push_back("Math.sqrt");
	math_func_list.push_back("Math.toRadians");
	math_func_list.push_back("Math.toDegrees");
	math_func_list.push_back("Math.ulp");

	trig_func_list.push_back("Math.acos");
	trig_func_list.push_back("Math.asin");
	trig_func_list.push_back("Math.atan");
	trig_func_list.push_back("Math.atan2");
	trig_func_list.push_back("Math.cos");
	trig_func_list.push_back("Math.cosh");
	trig_func_list.push_back("Math.sin");
	trig_func_list.push_back("Math.sinh");
	trig_func_list.push_back("Math.tan");
	trig_func_list.push_back("Math.tanh");

	log_func_list.push_back("Math.log");
	log_func_list.push_back("Math.log10");
	log_func_list.push_back("Math.log1p");

	cmplx_cyclomatic_list.push_back("if");
	cmplx_cyclomatic_list.push_back("case");
	cmplx_cyclomatic_list.push_back("while");
	cmplx_cyclomatic_list.push_back("for");
	cmplx_cyclomatic_list.push_back("catch");
	cmplx_cyclomatic_list.push_back("?");
}

/*!
* Constructs a CJavaJspCounter object.
*/
CJavaJspCounter::CJavaJspCounter()
{
	classtype = JAVA_JSP;
	language_name = "Java/JSP";

	file_extension.clear();
	file_extension.push_back(".*java");
}
