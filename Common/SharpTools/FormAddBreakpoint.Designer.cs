namespace SharpTools
{
	partial class FormAddBreakpoint
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if (disposing && (components != null))
			{
				components.Dispose();
			}
			base.Dispose(disposing);
		}

		#region Windows Form Designer generated code

		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			splitContainer1 = new SplitContainer();
			breakpointProps1 = new BreakpointProps();
			button1 = new Button();
			((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
			splitContainer1.Panel1.SuspendLayout();
			splitContainer1.Panel2.SuspendLayout();
			splitContainer1.SuspendLayout();
			SuspendLayout();
			// 
			// splitContainer1
			// 
			splitContainer1.Dock = DockStyle.Fill;
			splitContainer1.FixedPanel = FixedPanel.Panel2;
			splitContainer1.Location = new Point(0, 0);
			splitContainer1.Name = "splitContainer1";
			splitContainer1.Orientation = Orientation.Horizontal;
			// 
			// splitContainer1.Panel1
			// 
			splitContainer1.Panel1.Controls.Add(breakpointProps1);
			// 
			// splitContainer1.Panel2
			// 
			splitContainer1.Panel2.Controls.Add(button1);
			splitContainer1.Size = new Size(683, 392);
			splitContainer1.SplitterDistance = 337;
			splitContainer1.TabIndex = 0;
			// 
			// breakpointProps1
			// 
			breakpointProps1.Dock = DockStyle.Fill;
			breakpointProps1.Location = new Point(0, 0);
			breakpointProps1.Name = "breakpointProps1";
			breakpointProps1.Size = new Size(683, 337);
			breakpointProps1.TabIndex = 0;
			// 
			// button1
			// 
			button1.Anchor = AnchorStyles.Bottom | AnchorStyles.Right;
			button1.Location = new Point(586, 8);
			button1.Name = "button1";
			button1.Size = new Size(89, 34);
			button1.TabIndex = 0;
			button1.Text = "Add";
			button1.UseVisualStyleBackColor = true;
			button1.Click += button1_Click;
			// 
			// FormAddBreakpoint
			// 
			AutoScaleDimensions = new SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(683, 392);
			Controls.Add(splitContainer1);
			Name = "FormAddBreakpoint";
			Text = "Add Breakpoint";
			splitContainer1.Panel1.ResumeLayout(false);
			splitContainer1.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
			splitContainer1.ResumeLayout(false);
			ResumeLayout(false);
		}

		#endregion

		private SplitContainer splitContainer1;
		private Button button1;
		private BreakpointProps breakpointProps1;
	}
}