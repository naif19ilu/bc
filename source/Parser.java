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
		Token lastSeen = null;

		for (long i = 0; i < length; i++)
		{
			final char mnemonic = source[(int) i];

			if (lastSeen != null && lastSeen.getMnemonic() == mnemonic && lastSeen.hasFamily())
			{
				offsetline++;
				lastSeen.increaseFamilySize();
				continue;
			}

			Token token = null;
			switch (mnemonic)
			{
				case '+':
				case '-':
				case '<':
				case '>':
				case '.':
				case ',':  { token = new Token(numberline, offsetline, mnemonic); break; }
				case '[':  { token = handleOpening(); break; }
				case ']':  { token = handleClosing(); break; }
				case '\n': { numberline++; offsetline = 0; continue; }
				default:   { continue; }
			}

			offsetline++;
			lastSeen = token;
			stream.add(token);
		}

		if (!loopStack.isEmpty()) { reportAllUnclosedOnes(); }
		return stream;
	}

	private static Token handleOpening ()
	{
		final int foundAt = stream.size();

		Token token = new Token(numberline, offsetline, '[', foundAt);
		loopStack.push(foundAt);
		return token;
	}
	
	private static Token handleClosing ()
	{
		if (loopStack.size() == 0) { Fatal.prematureClosing(numberline, offsetline); }
		final int parnerIsAt = loopStack.pop();

		stream.get(parnerIsAt).setParnerPosition(stream.size());
		Token token = new Token(numberline, offsetline, '[', parnerIsAt);
		return token;
	}

	private static void reportAllUnclosedOnes ()
	{
		while (true)
		{
			final int at = loopStack.pop();
			Token err = stream.get(at);

			if (loopStack.size() == 0) { Fatal.unclosedLoop(err.getNumberline(), err.getOffsetline(), true); }
			else { Fatal.unclosedLoop(err.getNumberline(), err.getOffsetline(), false); }
			System.err.println("");
		}
	}
}
