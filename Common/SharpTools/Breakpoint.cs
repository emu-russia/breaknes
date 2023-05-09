using SharpTools;

public enum BreakpontTrigger
{
	High,
	Low,
	Z,
	Posedge,
	Negedge,
	VectorEqual,
}

public class Breakpoint
{
	public BreaksCore.DebugInfoType info_type;
	public BreaksCore.DebugInfoEntry info_entry;
	public bool enabled;
	public bool autoclear;
	public BreakpontTrigger trigger;
	public UInt32 vector_value;
	public UInt32 prev_value;
}
