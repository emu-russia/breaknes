using SharpTools;
using System.Text;

namespace Breaknes
{
	/// <summary>
	/// The Logging configuration dialog (issue #517).
	/// The sources and their categories are queried from BreaksCore (they are
	/// registered by the native side on board creation); the user's choices are
	/// written back into the BreaknesSettings object. The settings are applied to
	/// the native side by FormMain when the Settings dialog closes.
	/// </summary>
	public class FormLogging : Form
	{
		private readonly FormSettings.BreaknesSettings settings;

		private CheckBox chkEnabled;
		private CheckBox chkToFile;
		private TextBox txtFile;
		private Button btnBrowse;
		private CheckBox chkToStdout;
		private TreeView tree;
		private Button btnOk;
		private Button btnCancel;

		public FormLogging(FormSettings.BreaknesSettings settings)
		{
			this.settings = settings;

			Text = "Logging";
			StartPosition = FormStartPosition.CenterParent;
			FormBorderStyle = FormBorderStyle.FixedDialog;
			MaximizeBox = false;
			MinimizeBox = false;
			ClientSize = new Size(500, 430);

			BuildUi();
		}

		private void BuildUi()
		{
			int x = 12;
			int w = 476;

			chkEnabled = new CheckBox
			{
				Text = "Enable logging",
				Location = new Point(x, 12),
				Size = new Size(200, 24),
			};

			chkToFile = new CheckBox
			{
				Text = "Write log to file:",
				Location = new Point(x, 42),
				Size = new Size(130, 24),
			};

			txtFile = new TextBox
			{
				Location = new Point(142, 43),
				Size = new Size(w - 142 - 32, 23),
			};

			btnBrowse = new Button
			{
				Text = "...",
				Location = new Point(w - 18, 42),
				Size = new Size(28, 23),
			};
			btnBrowse.Click += BtnBrowse_Click;

			chkToStdout = new CheckBox
			{
				Text = "Write log to stdout (debug console)",
				Location = new Point(x, 72),
				Size = new Size(300, 24),
			};

			Label lblSources = new Label
			{
				Text = "Log sources and categories:",
				Location = new Point(x, 104),
				Size = new Size(300, 20),
			};

			tree = new TreeView
			{
				Location = new Point(x, 126),
				Size = new Size(w, 250),
				CheckBoxes = true,
				HideSelection = false,
				FullRowSelect = true,
				Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right | AnchorStyles.Bottom,
			};
			tree.AfterCheck += Tree_AfterCheck;

			btnOk = new Button
			{
				Text = "OK",
				Location = new Point(w - 174, 392),
				Size = new Size(80, 28),
				DialogResult = DialogResult.OK,
				Anchor = AnchorStyles.Bottom | AnchorStyles.Right,
			};
			btnOk.Click += BtnOk_Click;

			btnCancel = new Button
			{
				Text = "Cancel",
				Location = new Point(w - 88, 392),
				Size = new Size(80, 28),
				DialogResult = DialogResult.Cancel,
				Anchor = AnchorStyles.Bottom | AnchorStyles.Right,
			};

			Controls.AddRange(new Control[] { chkEnabled, chkToFile, txtFile, btnBrowse, chkToStdout, lblSources, tree, btnOk, btnCancel });
			AcceptButton = btnOk;
			CancelButton = btnCancel;
		}

		private void BtnBrowse_Click(object? sender, EventArgs e)
		{
			using SaveFileDialog dlg = new SaveFileDialog();
			dlg.Title = "Select the log file";
			dlg.Filter = "Log files (*.log)|*.log|All files (*.*)|*.*";
			dlg.FileName = txtFile.Text;
			if (dlg.ShowDialog(this) == DialogResult.OK)
			{
				txtFile.Text = dlg.FileName;
			}
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			chkEnabled.Checked = settings.LogEnabled;
			chkToFile.Checked = settings.LogToFile;
			txtFile.Text = settings.LogFile;
			chkToStdout.Checked = settings.LogToStdout;

			ReloadTree();
		}

		private void ReloadTree()
		{
			tree.BeginUpdate();
			tree.Nodes.Clear();

			int sourceCount = BreaksCore.GetLogSourceCount();
			ulong sourceMask = settings.LogSourceMask;

			for (int src = 0; src < sourceCount; src++)
			{
				StringBuilder nameBuf = new StringBuilder(64);
				BreaksCore.GetLogSourceName(src, nameBuf, 64);
				string srcName = nameBuf.ToString();

				TreeNode srcNode = new TreeNode(srcName);
				srcNode.Tag = src;
				srcNode.Checked = (sourceMask & (1UL << src)) != 0;

				ulong catMask = FormSettings.GetLogCategoryMask(settings, src);
				int catCount = BreaksCore.GetLogCategoryCount(src);

				for (int cat = 0; cat < catCount; cat++)
				{
					ulong bit = BreaksCore.GetLogCategoryBit(src, cat);

					StringBuilder catNameBuf = new StringBuilder(64);
					BreaksCore.GetLogCategoryName(src, cat, catNameBuf, 64);

					TreeNode catNode = new TreeNode(catNameBuf.ToString());
					catNode.Tag = bit;
					catNode.Checked = (catMask & bit) != 0;
					srcNode.Nodes.Add(catNode);
				}

				srcNode.Expand();
				tree.Nodes.Add(srcNode);
			}

			tree.EndUpdate();
		}

		private void Tree_AfterCheck(object? sender, TreeViewEventArgs e)
		{
			// Checking a source checks all its categories (they only make sense together).
			if (e.Node.Tag is int && e.Node.Nodes.Count != 0)
			{
				bool check = e.Node.Checked;
				foreach (TreeNode cat in e.Node.Nodes)
				{
					cat.Checked = check;
				}
			}
		}

		private void BtnOk_Click(object? sender, EventArgs e)
		{
			ulong sourceMask = 0;
			Dictionary<int, ulong> masks = new();

			foreach (TreeNode srcNode in tree.Nodes)
			{
				int src = (int)srcNode.Tag!;

				if (srcNode.Checked)
				{
					sourceMask |= 1UL << src;
				}

				ulong catMask = 0;
				foreach (TreeNode catNode in srcNode.Nodes)
				{
					if (catNode.Checked)
					{
						catMask |= (ulong)catNode.Tag!;
					}
				}
				masks[src] = catMask;
			}

			settings.LogEnabled = chkEnabled.Checked;
			settings.LogToFile = chkToFile.Checked;
			settings.LogFile = txtFile.Text;
			settings.LogToStdout = chkToStdout.Checked;
			settings.LogSourceMask = sourceMask;
			FormSettings.SetLogCategoryMasks(settings, masks);

			DialogResult = DialogResult.OK;
			Close();
		}
	}
}
