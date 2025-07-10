public class Fatal
{
	public static void fileDoesNotExist (final String filename)
	{
		final String fmt = String.format(
			"bc: \u001b[31mfatal\u001b[0m: %s file does not exist\n" +
			"compilation terminated.",
			filename
		);
		System.err.println(fmt);
		System.exit(1);
	}

	public static void cannotReadFile (final String filename)
	{
		final String fmt = String.format(
			"bc: \u001b[31mfatal\u001b[0m: %s file cannot be read\n" +
			"compilation terminated.",
			filename
		);
		System.err.println(fmt);
		System.exit(1);
	}

	public static void InputOutputInt (final String filename)
	{
		final String fmt = String.format(
			"bc: \u001b[31mfatal\u001b[0m: input/output interrupted error\n" +
			"while working with %s file: aborting",
			filename
		);
		System.err.println(fmt);
		System.exit(1);
	}

	public static void uncompletedRead (final String filename, final long read, final long outof)
	{
		final String fmt = String.format(
			"bc: \u001b[31mfatal\u001b[0m: cannot read 100% of %s file\n" +
			"only %d out of %d bytes were read; aborting",
			filename,
			read,
			outof
		);
		System.err.println(fmt);
		System.exit(1);
	}

	public static void prematureClosing (final int numberline, final int offsetline)
	{
		final String fmt = String.format(
			"bc: \u001b[31mfatal\u001b[0m: premature loop closing\n" +
			"byte on %d line with an offset of %d has no opening-parner\n" +
			"check all loops are balanced!",
			numberline,
			offsetline
		);
		System.err.println(fmt);
		System.exit(1);
	}

	public static void unclosedLoop (final int numberline, final int offsetline, final boolean exit)
	{
		final String fmt = String.format(
			"bc: \u001b[31mfatal\u001b[0m: unclosed loop!\n" +
			"byte on %d line with an offset of %d has no closing-parner\n" +
			"check all loops are balanced!",
			numberline,
			offsetline
		);
		System.err.println(fmt);
		if (exit) { System.exit(1); }
	}
}
