
import jxa.*;

public class Main
{
	final private static JxaFlag flags[] =
	{
		new JxaFlag("compile", 'c', JxaFlag.FlagArg.YES,           "compiling source"),
		new JxaFlag("output",  'o', JxaFlag.FlagArg.MAY, "a.out",  "place the output in the file provided (a.out by default)"),
		new JxaFlag("arch",    'a', JxaFlag.FlagArg.MAY, "x86_64", "arch to be used (x64 or ARM) (x64 by default)")
	};

	public static void main (String args[])
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
