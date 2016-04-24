#Best nmake tutorial: http://www.bogotobogo.com/cplusplus/make.php
#INCS="path to headers\include"

# Specify the location to my SDK libs (e.g. DbgHelp.Lib)
LIBS="C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\Lib" 

mybuild: main.obj
	cl /Zi /o vtable.exe main.obj /link  \
		/DEBUG /LIBPATH:$(LIBS) DbgHelp.Lib 

main.obj: main.cpp
	cl /c main.cpp

all:mybuild

clean:
   del *.exe *.obj

#------------------------------------------------------------------
# If creating multiple obj files:
#mybuild: main.obj foo.obj
#	cl /o myTest.exe main.obj foo.obj /link  \
#		/LIBPATH:$(LIBS) myLib.lib /NODEFAULTLIB:libcmt.lib

# To create the 2nd target: foo.obj
#foo.obj: foo.cpp
#	cl /c /EHsc foo.cpp -I $(INCS)
 

