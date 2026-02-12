{
	// https://code.visualstudio.com/docs/editor/userdefinedsnippets
	// Place your global snippets here. Each snippet is defined under a snippet name and has a scope, prefix, body and 
	// description. Add comma separated ids of the languages where the snippet is applicable in the scope field. If scope 
	// is left empty or omitted, the snippet gets applied to all languages. The prefix is what is 
	// used to trigger the snippet and the body will be expanded and inserted. Possible variables are: 
	// $1, $2 for tab stops, $0 for the final cursor position, and ${1:label}, ${2:another} for placeholders. 
	// Placeholders with the same ids are connected.
	// Example:
	// "Print to console": {
	// 	"scope": "javascript,typescript",
	// 	"prefix": "log",
	// 	"body": [
	// 		"console.log('$1');",
	// 		"$2"
	// 	],
	// 	"description": "Log output to console"
	// }
	"cfile header" : {
		"prefix": "cfh",
		"body": [
			"/**",
			"  ******************************************************************************",
			"  * @file    : ${TM_FILENAME}",
			"  * @brief   : $1",
			"  * @author  : MYMEDIA Co., Ltd.",
			"  * @version : V1.0.0",
			"  * @date    : ${CURRENT_YEAR} : ${CURRENT_MONTH} : ${CURRENT_DATE}",
			"  ******************************************************************************",
			"  */"
		]
	},	
	"cfile function start": {
		"prefix": "cfs",
		"body": [
			"/**",
			"  ******************************************************************************",
			"  * @brief          : ${1:Main function.}",
			"  * @par Parameters :",
			"  * ${2:None}",
			"  * @retval         : ${3:void None}",
			"  * @par Required preconditions:",
			"  * ${4:None}",
			"  ******************************************************************************",
			"  */"
		],
		"description": "cfile function start"
	},
	"cfile start define": {
		"prefix": "csd",
		"body": [
			"/* Includes ------------------------------------------------------------------*/",
			"#include \"stm8s.h\"",
			"/* Private typedef -----------------------------------------------------------*/",
			"/* Private define ------------------------------------------------------------*/",
			"/* Private macro -------------------------------------------------------------*/",
			"/* Private variables ---------------------------------------------------------*/",
			"/* Private function prototypes -----------------------------------------------*/",
			"/* Private functions ---------------------------------------------------------*/",
			"/* Global variables ----------------------------------------------------------*/",
			"/* Public functions ----------------------------------------------------------*/"
		],
		"description": "cfile start define"
	},
	"pfile header" : {
		"scope": "python",
		"prefix": "pfh",
		"body": [
			"#",
			"# filename : ${TM_FILENAME}",
			"# date     : ${CURRENT_YEAR} : ${CURRENT_MONTH} : ${CURRENT_DATE}",
			"# by yooaroma",
			"#"
		]
	}
}