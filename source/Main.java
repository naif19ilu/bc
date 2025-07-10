import jxa.*;
import java.io.File;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.FileReader;

public class Main
{
	private static long sourceLength;
	private static char source[];

	final private static JxaFlag flags[] =
	{
		new JxaFlag("compile", 'c', JxaFlag.FlagArg.YES,           "compiling source"),
		new JxaFlag("output",  'o', JxaFlag.FlagArg.MAY, "a.out",  "place the output in the file provided (a.out by default)"),
		new JxaFlag("arch",    'a', JxaFlag.FlagArg.MAY, "x86_64", "arch to be used (x64 or ARM) (x64 by default)")
	};

	private static void handleFile (final String filename)
	{
		File file = new File(filename);

		if (!file.exists())  { Fatal.fileDoesNotExist(filename); }
		if (!file.canRead()) { Fatal.cannotReadFile(filename); }

		try
		{
			FileReader reader = new FileReader(file);
			sourceLength = file.length();
			source = new char[(int) sourceLength];

			final int bytesRead = reader.read(source, 0, (int) sourceLength);

			if (bytesRead != sourceLength)
			{
				Fatal.uncompletedRead(filename, bytesRead, sourceLength);
			}
			reader.close();
		}
		catch (IOException why)
		{
			if (why instanceof FileNotFoundException)
			{
				Fatal.fileDoesNotExist(filename);
			}
			Fatal.InputOutputInt(filename);
		}
	}


	public static void main (String args[])
	{
		Jxa.parse("bc", args, flags);

		final String compile = flags[0].getArgument();
		if (compile.isEmpty())
		{
			JxaDoc.printUsage("bc", flags);
			System.exit(0);
		}

		handleFile(compile);

		System.out.println(source);
	}
}
