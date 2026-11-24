import ghidra.app.script.GhidraScript;
import ghidra.app.decompiler.*;
import ghidra.program.model.listing.*;
import java.io.*;

public class DecompAll extends GhidraScript {
    @Override
    public void run() throws Exception {
        DecompInterface di = new DecompInterface();
        di.openProgram(currentProgram);
        PrintWriter pw = new PrintWriter(new FileWriter("/tmp/decomp_out.c"));
        FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
        while (it.hasNext()) {
            Function f = it.next();
            DecompileResults res = di.decompileFunction(f, 60, monitor);
            pw.println("// ==== func @ 0x" + Long.toHexString(f.getEntryPoint().getOffset()) + " ====");
            if (res.decompileCompleted() && res.getDecompiledFunction() != null)
                pw.println(res.getDecompiledFunction().getC());
            else
                pw.println("// decompile failed");
        }
        pw.close();
        println("done");
    }
}
