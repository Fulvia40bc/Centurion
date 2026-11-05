import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.app.decompiler.*;

public class FindMgr extends GhidraScript {
    @Override
    public void run() throws Exception {
        AddressSpace space = currentProgram.getAddressFactory().getDefaultAddressSpace();
        Address[] targets = {
            space.getAddress(0xE6F4), space.getAddress(0xE6F6),   // dispatch far ptr
            space.getAddress(0x1B065),                               // "OVL0.OVL" string
            space.getAddress(0x1AEE5)                                // "Overlay Loader" string
        };
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        for (Address t : targets) {
            println("=== refs to " + t + " ===");
            ReferenceIterator it = currentProgram.getReferenceManager().getReferencesTo(t);
            int n = 0;
            while (it.hasNext() && n < 6) {
                Reference r = it.next();
                n++;
                Function f = getFunctionContaining(r.getFromAddress());
                println("  from 0x" + r.getFromAddress() + " func=" + (f != null ? f.getEntryPoint().toString() : "?"));
                if (f != null) {
                    DecompileResults res = di.decompileFunction(f, 120, monitor);
                    if (res.decompileCompleted() && res.getDecompiledFunction() != null) {
                        String c = res.getDecompiledFunction().getC();
                        println(c.length() > 3000 ? c.substring(0, 3000) : c);
                    }
                }
            }
            if (n == 0) println("  (none)");
        }
    }
}
