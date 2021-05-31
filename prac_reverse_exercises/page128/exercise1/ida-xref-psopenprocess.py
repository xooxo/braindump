
print "hello"
xrefTo = 0x14049D60C 
print(GetFunctionName(xrefTo)) 
fil = open("C:\\Users\\Lenovo\\Desktop\\curiosity\\prac_reverse_exercises\\page128\\exercise1\\xrefs_from_psopenprocess.txt","w")
for addr in XrefsFrom(xrefTo,ida_xref.XREF_ALL):
    print("Function Name: %s\n\t --> Function Address: %s" %(GetFunctionName(addr.frm),hex(addr.frm)))
    fil.write("Function Name: %s\n\t --> Function Address: %s\n" %(GetFunctionName(addr.frm),hex(addr.frm)))
	#print hex(addr.frm)

fil.close()