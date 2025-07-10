import jxa.*;
import java.io.File;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileOutputStream;
import java.util.List;

class Elf
{
}

class Compiler
{


}

public class Main
{
	private static long sourceLength;
	private static char source[];

	private static int memSize = 30000;

	final private static JxaFlag flags[] =
	{
		new JxaFlag("compile", 'c', JxaFlag.FlagArg.YES,           "compiling source"),
		new JxaFlag("output",  'o', JxaFlag.FlagArg.MAY, "a.out",  "place the output in the file provided (a.out by default)"),
		new JxaFlag("memory",  'm', JxaFlag.FlagArg.MAY, "30000",  "memory size (in bytes 30000 by default)")
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
			try { reader.close(); }
			catch (IOException why) { Fatal.InputOutputInt(filename); }

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

		try     { memSize = Integer.parseInt(flags[3].getArgument()); }
		finally { memSize = 30000; }

		handleFile(compile);
	}
}
