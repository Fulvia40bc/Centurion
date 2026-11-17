import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.*;
import ghidra.program.model.listing.*;

public class MarkEntry extends GhidraScript {
    @Override
    public void run() throws Exception {
        Address entry = currentProgram.getImageBase().add(0xAAC2);
        createFunction(entry, "entry");
        disassemble(entry);
        println("marked entry at " + entry);
    }
}
