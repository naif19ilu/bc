/*
 * bc - Brainfuck compiler
 * Jul 9, 2025
 * Main file
 */

import jxa.*;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

class Token
{
	private final int  numberLine;
	private final int  lineOffset;
	private       int  helper;
	private final char mnemonic;
	
	public Token (int nl, int off, int help, char ch)
	{
		this.numberLine = nl;
		this.lineOffset = off;
		this.helper     = help;
		this.mnemonic   = ch;
	}
	
	public char getMnemonic ()   { return this.mnemonic; }
	public int  getNumberLine () { return this.numberLine; }
	public int  getLineOffset () { return this.lineOffset; }
	public int  getHelper ()     { return this.helper; }
	
	public void increaseFamily ()   { this.helper++; }
	public void setParner (int pos) { this.helper = pos; }
}

public class Main
{
	final private static JxaFlag[] flags = new JxaFlag[] {
		new JxaFlag("compile", 'c', JxaFlag.FlagArg.YES, "file to be compiled"),
		new JxaFlag("output",  'o', JxaFlag.FlagArg.MAY, "a.out", "output filename (compiled file)"),
		new JxaFlag("arch",    'A', JxaFlag.FlagArg.MAY, "x64", "compile to (x64 | ARM)")
	};
	
	private static long fileLength;
	private static char sourceCode[];

	private static void readSourceCode (final String filename)
	{
		File file = new File(filename);
		if (!file.exists())
		{	
			Fatal.fileDoesNotExist(filename);
		}
		if (!file.canRead())
		{
			Fatal.fileCannotBeRead(filename);
		}

		FileReader reader;
		try
		{
			reader = new FileReader(file);
			fileLength = file.length();
			sourceCode = new char[(int) fileLength];
			reader.read(sourceCode);
			reader.close();
		}
		catch (IOException why)
		{
			if (why instanceof FileNotFoundException)
			{
				Fatal.fileDoesNotExist(filename);
			}
			Fatal.InputOutputInt();
		}	
	}
	
	public static void main(String[] args)
	{
		Jxa.parse("bc", args, flags);
		final String filename = flags[0].getArgument();

		if (filename.isEmpty()) { JxaDoc.printUsage("bc", flags); }
		readSourceCode(filename);
	}
}

