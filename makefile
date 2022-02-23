current:
	echo make pd_linux, pd_linux32, pd_nt, pd_darwin

clean: ; rm -f *.pd_linux *.o

# ----------------------- Windows-----------------------
# note; you will certainly have to edit the definition of VC to agree with
# whatever you've got installed on your machine:

VC="C:\Program FIles (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.16.27023"

pd_nt: sawteeth~.dll 

.SUFFIXES: .obj .dll

PDNTCFLAGS = /W3 /DNT /DPD /nologo

PDNTINCLUDE = /I. /I\tcl\include /I..\..\src /I$(VC)\include

PDNTLDIR = $(VC)\lib\x64
PDNTLIB = $(PDNTLDIR)\libcmt.lib \
	$(PDNTLDIR)\oldnames.lib \
	..\..\bin\pd.lib 

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:$*_setup $*.obj $(PDNTLIB)

# override explicitly for tilde objects like this:


sawteeth~.dll: sawteeth~.c; 
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:sawteeth_tilde_setup $*.obj $(PDNTLIB)






