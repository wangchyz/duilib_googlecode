cmake_minimum_required(VERSION 2.6)
# set(CMAKE_RC_COMPILER "D:/mingw32/bin/windres.exe")
add_library(DuiLib SHARED
stb_image.c            
StdAfx.cpp                
UIActiveX.cpp             
UIBase.cpp                
UICombo.cpp               
UICommonControls.cpp      
UIContainer.cpp           
UIControl.cpp             
UIDelegate.cpp            
UIDlgBuilder.cpp          
UIList.cpp                
UIManager.cpp             
UIMarkup.cpp              
UIRender.cpp              
UIRichEdit.cpp            
UIlib.cpp                 
XUnzip.cpp                )
target_link_libraries(DuiLib riched20 comctl32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w -fpermissive")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s")