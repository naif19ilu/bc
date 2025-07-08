/*
 * jxa - Java executable arguments parser
 * Jul 5, 2025
 * Error handling
 */

package jxa;

public final class JxaFatal
{
	private String pname;

	public JxaFatal (String pname)
	{
		this.pname = pname;
	}
	
	public void duplicatedId (int first, int second, JxaFlag[] flags)
	{
		final String msg = String.format(
			"jxa::%s::error: duplicated id!\n" +
			"  both '%s' and '%s' flags share same id '%c'\n",
			this.pname,
			flags[first].getLongName(),
			flags[second].getLongName(),
			flags[second].getShortName()
		);
		System.err.print(msg);
		System.exit(1);
	}
	
	public void invalidId (JxaFlag flag)
	{
		final String msg = String.format(
			"jxa::%s::error: invalid id provided\n" +
			"  the '%s' flag has an invalid id (%c)\n",
			this.pname,
			flag.getLongName(),
			flag.getShortName()
		);
		System.err.print(msg);
		System.exit(1);
	}
	
	public void undefinedFlag (String given)
	{
		final String msg = String.format(
			"jxa::%s::error: undefined flag provided\n" +
			"  the program cannot recognize '--%s' as a flag ,':|\n",
			this.pname,
			given
		);
		System.err.print(msg);
		System.exit(1);
	}

	public void undefinedFlag (char given)
	{
		final String msg = String.format(
			"jxa::%s::error: undefined flag provided\n" +
			"  the program cannot recognize '-%c' as a flag ,':|\n",
			this.pname,
			given
		);
		System.err.print(msg);
		System.exit(1);
	}
	
	public void groupedArgedFlags (String given, char first, char second)
	{
		final String msg = String.format(
			"jxa::%s::error: more than one flag needs argument in this group\n" +
			"  the program found '%s'; Both '-%c' and '-%c' takes argument\n" +
			"  better do: -%c <arg> -%c <arg> ...\n",
			this.pname,
			given,
			first,
			second,
			first,
			second
		);
		System.err.print(msg);
		System.exit(1);
	}
	
	public void missingArgument (JxaFlag flag)
	{
		final String msg = String.format(
			"jxa::%s::error: flag is missing its argument\n" +
			"  the program found that '--%s' has no argument, please provide it\n",
			this.pname,
			flag.getLongName()
		);
		System.err.print(msg);
		System.exit(1);
	}
	
	public void duplicatedName (JxaFlag flag)
	{	
		final String msg = String.format(
			"jxa::%s::error: longname appears more than once\n" +
			"  the program found that '--%s' longname is repeated\n",
			this.pname,
			flag.getLongName()
		);
		System.err.print(msg);
		System.exit(1);
	}
	
	public void noNeedOfArg (String flagname)
	{	
		final String msg = String.format(
			"jxa::%s::error: giving unnecessary argument\n" +
			"  the program found that '--%s' is taking an argument when it does not ask for it\n",
			this.pname,
			flagname
		);
		System.err.print(msg);
		System.exit(1);
	}
	
	public void unsupportedStdin ()
	{
		System.out.printf("jxa::%s::error: '-' UNIX option is not supported, use a shell command instead! Sorry\n", this.pname);
		System.exit(1);
	}
}