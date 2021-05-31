import sys
import r2pipe
import frida

binary = "/bin/gcc"

r2 = r2pipe.open(binary)
r2.cmd("aaaa")
funcAddrsOut = r2.cmd("fs functions; f")
funcAddrsOut = funcAddrsOut.split()

funcAddrs = []
scripts = []

def makeInterceptorScript(addr,session):
    script = session.create_script("""Interceptor.attach(ptr('%s'),{onEnter: function(args){ send(args[0].toInt32()); }})""" %(int(addr,16)))
    return script

pid = frida.spawn(binary)
process = frida.attach(pid)

for i in range(0,len(funcAddrsOut),3):
    funcAddrs.append(funcAddrsOut[i])


for i in range(len(funcAddrs)):
    scripts.append(makeInterceptorScript(funcAddrs[i],process))


def on_message(message,data):
    print("Frida has message for you! :)\n{}\n".format(message))


for i in range(len(scripts)):
    print("hooking %s" %(funcAddrs[i]))
    scripts[i].on("message",on_message)
    scripts[i].load()
sys.stdin.read()

