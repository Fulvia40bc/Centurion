import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;
import ghidra.program.model.symbol.*;
import ghidra.app.decompiler.*;

public class FindRefs extends GhidraScript {
    @Override
    public void run() throws Exception {
        Address target = currentProgram.getAddressFactory().getDefaultAddressSpace().getAddress(0x100C4);
        ReferenceIterator it = currentProgram.getReferenceManager().getReferencesTo(target);
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        while (it.hasNext()) {
            Reference r = it.next();
            Function f = getFunctionContaining(r.getFromAddress());
            println("ref to 0x100C4 from 0x" + r.getFromAddress() + " in func " + (f != null ? f.getName() + " @ " + f.getEntryPoint() : "none"));
            if (f != null) {
                DecompileResults res = di.decompileFunction(f, 90, monitor);
                if (res.decompileCompleted())
                    println(res.getDecompiledFunction().getC());
            }
        }
    }
}
