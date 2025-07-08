/*
 * bc - Brainfuck compiler
 * Jul 7, 2025
 * Main file
 */

import jxa.*;

public class Main
{
	final private static JxaFlag[] flags = new JxaFlag[] {
		new JxaFlag("compile", 'c', JxaFlag.FlagArg.YES, "file to be compiled"),
		new JxaFlag("output",  'o', JxaFlag.FlagArg.MAY, "a.out", "output filename (compiled file)"),
		new JxaFlag("arch",    'A', JxaFlag.FlagArg.MAY, "x64", "compile to (x64 | ARM)")
	};
	
	public static void main(String[] args)
	{
		Jxa.parse("bc", args, flags);
		if (flags[0].getArgument().isEmpty())
		{
			JxaDoc.printUsage("bc", flags);
		}
		System.out.printf("compile: %s\n", flags[0].getArgument());
	}
}

