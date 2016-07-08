set breakpoint pending on
exec-file ../../../../omnetpp-4.6/bin/opp_run
## for Windows (separator is ';')
set args -l ../../../src/inet -n ../../../examples;../../../src -u Cmdenv -f Mixed.ini -c drr -r 0
# ## for Linux (separator is ':')
# set args -l ../../../src/inet -n ../../../examples:../../../src -u Cmdenv -f Mixed.ini -c drr -r 0
tbreak VLANTagger2::initialize
tbreak VLANTagger2::handleMessage
break VLANTagger2::requestPacket
break VLANTagger2.cc:201
# break VLANTagger2.cc:218
# display messageLength
# display payloadLength
# display numSent
# display currentTime
run
