/* bc - brainfuck compiler
 * 12 Jul, 2025
 * Main file
 */
import jxa.*;

public class Main
{
	private static JxaFlag[] flags = new JxaFlag[]
	{
		new JxaFlag("compile", 'c', JxaFlag.FlagArg.YES,          "brainfuck source to be compiled"),
		new JxaFlag("output",  'o', JxaFlag.FlagArg.MAY, "a.out", "output filename (a.out by default)"),
		new JxaFlag("source",  'S', JxaFlag.FlagArg.MAY, "a.s",   "produce assembly source and name it (a.s by default)"),
	};
	
	public static void main(String[] args)
	{
		Jxa.parse("bc", args, flags);

		final String compile = flags[0].getArgument();
		if (compile.isEmpty())
		{
			JxaDoc.printUsage("bc", flags);
			System.exit(0);
		}
		
		
	}
}
