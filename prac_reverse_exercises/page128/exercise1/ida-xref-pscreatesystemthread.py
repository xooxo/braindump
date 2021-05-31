
print "hello"
print(GetFunctionName(0x140440710))
fil = open("C:\\Users\\Lenovo\\Desktop\\curiosity\\prac_reverse_exercises\\page128\\exercise1\\xrefs_to_pscreatesystemthread.txt","w")
for addr in XrefsTo(0x140440710,ida_xref.XREF_ALL):
    print("Function Name: %s\n\t --> Function Address: %s" %(GetFunctionName(addr.frm),hex(addr.frm)))
    fil.write("Function Name: %s\n\t --> Function Address: %s\n" %(GetFunctionName(addr.frm),hex(addr.frm)))
	#print hex(addr.frm)

fil.close()