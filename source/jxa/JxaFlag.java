/*
 * jxa - Java executable arguments parser
 * Jul 4, 2025
 * Defines a flag and its components
 */

package jxa;

public final class JxaFlag
{
	public static enum FlagArg
	{
		YES,
		NON,
		MAY
	};
	
	private final String  longName;
	private final char    shortName;
	private final FlagArg needs;
	private final String  description;

	private boolean seen     = false;
	private String  argument = "";
	
	public JxaFlag (String longName, char shortName, FlagArg needs, String description)
	{
		this.longName    = longName;
		this.needs       = needs;
		this.shortName   = shortName;
		this.description = description;
	}
	
	public JxaFlag (String longName, char shortName, FlagArg needs, String defArg, String description)
	{
		this.longName    = longName;
		this.needs       = needs;
		this.shortName   = shortName;
		this.argument    = defArg;
		this.description = description;
	}
	
	public String getLongName () { return this.longName; }
	public char getShortName ()  { return this.shortName; }
	public FlagArg getNeeds ()   { return this.needs; }
	public String getArgument () { return this.argument; }
	public boolean getSeen ()    { return this.seen; }
	public String getDesc ()     { return this.description; }
	
	public void setArgument (String arg) { this.argument = arg; }
	public void setSeen (boolean seen) { this.seen = seen; }
}
