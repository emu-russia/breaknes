namespace Breaknes
{
	partial class FormBindingDendyController
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
			ListViewItem listViewItem1 = new ListViewItem("Up");
			ListViewItem listViewItem2 = new ListViewItem("Down");
			ListViewItem listViewItem3 = new ListViewItem("Left");
			ListViewItem listViewItem4 = new ListViewItem("Right");
			ListViewItem listViewItem5 = new ListViewItem("Device Key");
			ListViewItem listViewItem6 = new ListViewItem("Device Key");
			tabControl1 = new TabControl();
			tabPage1 = new TabPage();
			listView1 = new ListView();
			splitContainer1 = new SplitContainer();
			splitContainer2 = new SplitContainer();
			tabControl2 = new TabControl();
			tabPage2 = new TabPage();
			listView2 = new ListView();
			button4 = new Button();
			button3 = new Button();
			button2 = new Button();
			button1 = new Button();
			tabControl1.SuspendLayout();
			tabPage1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
			splitContainer1.Panel1.SuspendLayout();
			splitContainer1.Panel2.SuspendLayout();
			splitContainer1.SuspendLayout();
			((System.ComponentModel.ISupportInitialize)splitContainer2).BeginInit();
			splitContainer2.Panel1.SuspendLayout();
			splitContainer2.Panel2.SuspendLayout();
			splitContainer2.SuspendLayout();
			tabControl2.SuspendLayout();
			tabPage2.SuspendLayout();
			SuspendLayout();
			// 
			// tabControl1
			// 
			tabControl1.Controls.Add(tabPage1);
			tabControl1.Dock = DockStyle.Fill;
			tabControl1.Location = new Point(0, 0);
			tabControl1.Name = "tabControl1";
			tabControl1.SelectedIndex = 0;
			tabControl1.Size = new Size(374, 402);
			tabControl1.TabIndex = 0;
			// 
			// tabPage1
			// 
			tabPage1.Controls.Add(listView1);
			tabPage1.Location = new Point(4, 24);
			tabPage1.Name = "tabPage1";
			tabPage1.Padding = new Padding(3);
			tabPage1.Size = new Size(366, 374);
			tabPage1.TabIndex = 0;
			tabPage1.Text = "Actuators";
			tabPage1.UseVisualStyleBackColor = true;
			// 
			// listView1
			// 
			listView1.Dock = DockStyle.Fill;
			listView1.FullRowSelect = true;
			listView1.Items.AddRange(new ListViewItem[] { listViewItem1, listViewItem2, listViewItem3, listViewItem4 });
			listView1.Location = new Point(3, 3);
			listView1.MultiSelect = false;
			listView1.Name = "listView1";
			listView1.Size = new Size(360, 368);
			listView1.TabIndex = 0;
			listView1.UseCompatibleStateImageBehavior = false;
			listView1.View = View.List;
			listView1.Click += listView1_Click;
			// 
			// splitContainer1
			// 
			splitContainer1.Dock = DockStyle.Fill;
			splitContainer1.Location = new Point(0, 0);
			splitContainer1.Name = "splitContainer1";
			splitContainer1.Orientation = Orientation.Horizontal;
			// 
			// splitContainer1.Panel1
			// 
			splitContainer1.Panel1.Controls.Add(splitContainer2);
			// 
			// splitContainer1.Panel2
			// 
			splitContainer1.Panel2.Controls.Add(button4);
			splitContainer1.Panel2.Controls.Add(button3);
			splitContainer1.Panel2.Controls.Add(button2);
			splitContainer1.Panel2.Controls.Add(button1);
			splitContainer1.Size = new Size(801, 476);
			splitContainer1.SplitterDistance = 402;
			splitContainer1.TabIndex = 4;
			// 
			// splitContainer2
			// 
			splitContainer2.Dock = DockStyle.Fill;
			splitContainer2.Location = new Point(0, 0);
			splitContainer2.Name = "splitContainer2";
			// 
			// splitContainer2.Panel1
			// 
			splitContainer2.Panel1.Controls.Add(tabControl1);
			// 
			// splitContainer2.Panel2
			// 
			splitContainer2.Panel2.Controls.Add(tabControl2);
			splitContainer2.Size = new Size(801, 402);
			splitContainer2.SplitterDistance = 374;
			splitContainer2.TabIndex = 0;
			// 
			// tabControl2
			// 
			tabControl2.Controls.Add(tabPage2);
			tabControl2.Dock = DockStyle.Fill;
			tabControl2.Location = new Point(0, 0);
			tabControl2.Name = "tabControl2";
			tabControl2.SelectedIndex = 0;
			tabControl2.Size = new Size(423, 402);
			tabControl2.TabIndex = 0;
			// 
			// tabPage2
			// 
			tabPage2.Controls.Add(listView2);
			tabPage2.Location = new Point(4, 24);
			tabPage2.Name = "tabPage2";
			tabPage2.Padding = new Padding(3);
			tabPage2.Size = new Size(415, 374);
			tabPage2.TabIndex = 0;
			tabPage2.Text = "Bindings";
			tabPage2.UseVisualStyleBackColor = true;
			// 
			// listView2
			// 
			listView2.Dock = DockStyle.Fill;
			listView2.FullRowSelect = true;
			listView2.Items.AddRange(new ListViewItem[] { listViewItem5, listViewItem6 });
			listView2.Location = new Point(3, 3);
			listView2.MultiSelect = false;
			listView2.Name = "listView2";
			listView2.Size = new Size(409, 368);
			listView2.TabIndex = 1;
			listView2.UseCompatibleStateImageBehavior = false;
			listView2.View = View.List;
			// 
			// button4
			// 
			button4.Location = new Point(686, 17);
			button4.Name = "button4";
			button4.Size = new Size(102, 34);
			button4.TabIndex = 3;
			button4.Text = "Save";
			button4.UseVisualStyleBackColor = true;
			button4.Click += button4_Click;
			// 
			// button3
			// 
			button3.Location = new Point(228, 17);
			button3.Name = "button3";
			button3.Size = new Size(102, 34);
			button3.TabIndex = 2;
			button3.Text = "Clear Binding";
			button3.UseVisualStyleBackColor = true;
			button3.Click += button3_Click;
			// 
			// button2
			// 
			button2.Location = new Point(120, 17);
			button2.Name = "button2";
			button2.Size = new Size(102, 34);
			button2.TabIndex = 1;
			button2.Text = "Add Binding";
			button2.UseVisualStyleBackColor = true;
			button2.Click += button2_Click;
			// 
			// button1
			// 
			button1.Location = new Point(12, 17);
			button1.Name = "button1";
			button1.Size = new Size(102, 34);
			button1.TabIndex = 0;
			button1.Text = "Clear all";
			button1.UseVisualStyleBackColor = true;
			button1.Click += button1_Click;
			// 
			// FormBindingDendyController
			// 
			AutoScaleDimensions = new SizeF(7F, 15F);
			AutoScaleMode = AutoScaleMode.Font;
			ClientSize = new Size(801, 476);
			Controls.Add(splitContainer1);
			Name = "FormBindingDendyController";
			Text = "Dendy Controller Binding";
			Load += FormBindingDendyController_Load;
			tabControl1.ResumeLayout(false);
			tabPage1.ResumeLayout(false);
			splitContainer1.Panel1.ResumeLayout(false);
			splitContainer1.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
			splitContainer1.ResumeLayout(false);
			splitContainer2.Panel1.ResumeLayout(false);
			splitContainer2.Panel2.ResumeLayout(false);
			((System.ComponentModel.ISupportInitialize)splitContainer2).EndInit();
			splitContainer2.ResumeLayout(false);
			tabControl2.ResumeLayout(false);
			tabPage2.ResumeLayout(false);
			ResumeLayout(false);
		}

		#endregion

		private TabControl tabControl1;
		private TabPage tabPage1;
		private ListView listView1;
		private SplitContainer splitContainer1;
		private SplitContainer splitContainer2;
		private TabControl tabControl2;
		private TabPage tabPage2;
		private ListView listView2;
		private Button button4;
		private Button button3;
		private Button button2;
		private Button button1;
	}
}