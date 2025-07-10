import jxa.*;
import java.io.File;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileOutputStream;
import java.util.List;

class Elf
{
	private static enum Arch
	{
		x64,
		ARM
	};

	private static Arch             arch       = Arch.x64;
	private static FileOutputStream outFile    = null;
	private static String           filename   = null;
	private static int              fileOffset = 0;

	private static void createFile ()
	{
		try { outFile = new FileOutputStream(filename); }
		catch (IOException why) { Fatal.cannotCreateFile(filename); }
	}

	private static void writeRawBytes (final byte code[], int length)
	{
		try { outFile.write(code, 0, length); fileOffset += length; }
		catch (IOException why) { Fatal.InputOutputInt(filename); }
	}

	public static void produceElf (final String outputName, final String architecture, final int memSize, final List<Token> stream)
	{
		if (architecture.equals("ARM")) { arch = Arch.ARM; }
		filename = outputName;

		createFile();

		writeRawBytes(new byte[] {0x7f, 0x45, 0x4c, 0x46}, 4);                          /* This is a ELF file */
		writeRawBytes(new byte[] {0x02, 0x01, 0x01, 0x00}, 4);                          /* 64bit arch PC, little endian, version, UNIX System V ABI */
		writeRawBytes(new byte[] {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 8);  /* padding */

		try { outFile.close(); }
		catch (IOException why) { Fatal.InputOutputInt(outputName); }
	}
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
		new JxaFlag("arch",    'a', JxaFlag.FlagArg.MAY, "x86_64", "arch to be used (x64 or ARM) (x64 by default)"),
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
		Elf.produceElf(
			flags[1].getArgument(),
			flags[2].getArgument(),
			memSize,
			Parser.parse(source, sourceLength)
		);
	}
}
