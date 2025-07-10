public class Token
{
	final private int  numberline;
	final private int  offsetline;
	final private char mnemonic;

	/* Represents the amount of tokens with the same
	 * type found in a row, for example +++ would be 3 INC
	 */
	private int familySize     = 1;

	/* When a loop is opened/closed we need to know where its
	 * pair can be found, this is the index where we can access
	 * the pair
	 */
	private int parnerPosition = -1;

	public Token (final int numberline, final int offsetline, final char mnemonic)
	{
		this.numberline = numberline;
		this.offsetline = offsetline;
		this.mnemonic   = mnemonic;
	}

	/* This constructor is only used when a ']' is found since ']' can only exist if
	 * its parner ']' does too, so if ']' already exists we know before hand what is
	 * ']' parner's position
	 */
	public Token (final int numberline, final int offsetline, final char mnemonic, final int position)
	{
		this.numberline     = numberline;
		this.offsetline     = offsetline;
		this.mnemonic       = mnemonic;
		this.parnerPosition = position;
	}

	public int  getNumberline () { return this.numberline; }
	public int  getOffsetline () { return this.offsetline; }

	public char getMnemonic   () { return this.mnemonic; }

	public int getFamilySize  () { return this.familySize; }
	public int getParnerPos   () { return this.parnerPosition; }

	public void increaseFamilySize () { this.familySize++; }
	public void setParnerPosition (final int to)  { this.parnerPosition = to; }
}
