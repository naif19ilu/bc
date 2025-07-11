import jxa.*;
import java.io.File;
import java.io.IOException;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileOutputStream;
import java.util.List;
import java.util.ArrayList;

// prefix = not used
// rex    = whether i want to use 64 bit register
// opcode = what to do
// modR/M =
//    mod = addressing mode (register or memory)
//    reg = register operand (source)
//    r/m = register or mem op (destination)

class Compiler
{
	private static List<Byte> binary = new ArrayList<Byte>();
	private int noPages = 1;

	/* offset from the first instruction executed (plus one
	* in order to avoid overwiting the current instruction
	*/
	private static int codeOffset = 0x00000001;

	private static void writeBytes (final Integer[] inst)
	{
		for (int i = 0; i < inst.length; i++)
		{
			binary.add((Byte) inst[i].byteValue());
			codeOffset++;
		}
	}

	private static Integer[] getLilEndian (final int num)
	{
		return new Integer[] {
			((num >> 0 ) & 0xff),
			((num >> 8 ) & 0xff),
			((num >> 16) & 0xff),
			((num >> 24) & 0xff)
		};
	}

	private static void emmitPlus (final int times) { }
	private static void emmitMinus (final int times) { }
	private static void emmitNext (final int times) { }
	private static void emitPrevs (final int times) { }
	private static void emmitIO (final int times, final char op) { }
	private static void emmitLoopBeg (final int parnerPos) { }
	private static void emmitLoopCls (final int parnerPos) { }

	private static void dumpIntoFile (final String filename)
	{
	}

	public static void produceByteCode (final List<Token> stream, final String outname, final int memSize)
	{
		/* pushq %rbp
		 * movq  %rsp, %rbp
		 * subq  memSize, %rsp
		 * leaq  (%rbp), %r8
		 */
		writeBytes(new Integer[] {0x55, 0x48, 0x89, 0xe5, 0x48, 0x81, 0xec});
		writeBytes(getLilEndian(memSize));
		writeBytes(new Integer[] {0x4c, 0x8d, 0x45, 0x00});
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
		Compiler.produceByteCode(Parser.parse(source, sourceLength), compile, memSize);
	}
}
