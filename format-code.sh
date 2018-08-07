#!/bin/bash
set -e
clang-format --style "{ \
	Language: Cpp, \
	BasedOnStyle: LLVM, \
	AccessModifierOffset: -4, \
	AlignAfterOpenBracket: DontAlign, \
	AlignOperands: false, \
	AlignTrailingComments: false, \
	AllowShortBlocksOnASingleLine: false, \
	AllowShortFunctionsOnASingleLine: None, \
	AllowShortIfStatementsOnASingleLine: true, \
	AllowShortLoopsOnASingleLine: true, \
	AlwaysBreakTemplateDeclarations: false, \
	BreakBeforeBraces: Custom, \
	BraceWrapping: { AfterEnum: true, AfterFunction: true, BeforeElse: true }, \
	BreakConstructorInitializersBeforeComma: true, \
	ColumnLimit: 1000, \
	Cpp11BracedListStyle: true, \
	IndentWidth: 4, \
	PointerAlignment: Left, \
	SortIncludes: false, \
	SpaceAfterCStyleCast: true, \
	Standard: Cpp11, \
	TabWidth: 4, \
	UseTab: Never \
}" -i "$@"
clang-format --style "{ \
	Language: Cpp, \
	BasedOnStyle: LLVM, \
	AccessModifierOffset: -4, \
	AlignAfterOpenBracket: DontAlign, \
	AlignOperands: false, \
	AlignTrailingComments: false, \
	AllowShortBlocksOnASingleLine: false, \
	AllowShortFunctionsOnASingleLine: None, \
	AllowShortIfStatementsOnASingleLine: true, \
	AllowShortLoopsOnASingleLine: true, \
	AlwaysBreakTemplateDeclarations: false, \
	BreakBeforeBraces: Stroustrup, \
	BreakConstructorInitializersBeforeComma: true, \
	ColumnLimit: 0, \
	Cpp11BracedListStyle: true, \
	IndentWidth: 4, \
	PointerAlignment: Left, \
	SortIncludes: false, \
	SpaceAfterCStyleCast: true, \
	Standard: Cpp11, \
	TabWidth: 4, \
	UseTab: Never \
}" -i "$@"
