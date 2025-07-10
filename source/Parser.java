import java.util.List;
import java.util.ArrayList;
import java.util.Stack;

public class Parser
{
	private static int numberline = 1;
	private static int offsetline = 0;

	private static List<Token>    stream    = new ArrayList<Token>();
	private static Stack<Integer> loopStack = new Stack<Integer>();

	public static List<Token> parse (final char source[], final long length)
	{
		for (long i = 0; i < length; i++)
		{
			final char mnemonic = source[(int) i];

			switch (mnemonic)
			{
				case '+':
				case '-':
				case '<':
				case '>':
				case '.':
				case ',': { stream.add(new Token(numberline, offsetline, mnemonic)); break; }
				case '[': { handleOpening(); }
				case ']': { handleClosing(); }
			}
		}

		debug();
		return stream;
	}

	private static void handleOpening ()
	{
		final int foundAt = stream.size();
		stream.add(new Token(numberline, offsetline, '[', foundAt));
		loopStack.push(foundAt);
	}
	
	private static void handleClosing ()
	{
		if (loopStack.size() == 0) { Fatal.prematureClosing(numberline, offsetline); }
		final int parnerIsAt = loopStack.pop();

		stream.get(parnerIsAt).setParnerPosition(stream.size());
		stream.add(new Token(numberline, offsetline, '[', parnerIsAt));
	}

	private static void debug ()
	{
		for (int i = 0; i < stream.size(); i++)
		{
			System.out.printf("%c:%d\n", stream.get(i).getMnemonic(), stream.get(i).getFamilySize());
		}
	}
}
