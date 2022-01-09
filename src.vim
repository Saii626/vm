" Vim syntax file

if exists("b:current_syntax")
  finish
endif

syn keyword srcKeywords load add eq cmp_jmp halt debug_print var

syn region srcString start=+"+ end=+"+ end=+$+ 
syn region srcReg start="\[" end="\]"

syn match srcComment "#.*$" contains=NONE
syn match srcLabel ".*:$" contains=NONE
syn match srcControl ":\S+" contains=NONE

let b:current_syntax = "sr"

hi def link srcKeywords   Keyword
hi def link srcComment    Comment
hi def link srcString     String
hi def link srcLabel      PreProc
hi def link srcControl    Constant
hi def link srcReg        Constant


