
print "hello"
xrefToFrom = 0x140159080 
toFrom = "to"
print(GetFunctionName(xrefToFrom)) 
fil = open("C:\\Users\\Lenovo\\Desktop\\curiosity\\prac_reverse_exercises\\page128\\exercise1\\xrefs_%s_%s.txt"%(toFrom,GetFunctionName(xrefToFrom)),"w")

if toFrom == "from":
    funcXref = XrefsFrom
else:
    funcXref = XrefsTo
	
for addr in funcXref(xrefToFrom,ida_xref.XREF_ALL):
    print("Function Name: %s\n\t --> Function Address: %s" %(GetFunctionName(addr.frm),hex(addr.frm)))
    fil.write("Function Name: %s\n\t --> Function Address: %s\n" %(GetFunctionName(addr.frm),hex(addr.frm)))
	#print hex(addr.frm)

fil.close()