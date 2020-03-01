#pragma once

namespace ParcelSketch {

	using namespace System;
	using namespace System::Collections::Generic;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Linq;
	using namespace System::IO;
	using namespace System::Text;
	using namespace System::Drawing;

	using namespace System::Drawing::Drawing2D;

	/// <summary>
	/// Summary for MainForm
	/// </summary>
	public ref class MainForm : public System::Windows::Forms::Form
	{
	public:
		MainForm(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			this->MakeBackgroundGrid();
		}

		// The grid spacing.
		const System::Int32 grid_gap = 8;

		// The "size" of an object for mouse over purposes.
	private: const System::Int32 object_radius = 3;

			 // We're over an object if the distance squared
			 // between the mouse and the object is less than this.
	private: const System::Int32 over_dist_squared = object_radius * object_radius;

			 // The points that make up the line segments.
	private: List<Point>^ Pt1 = gcnew List<Point>();
	private: List<Point>^ Pt2 = gcnew List<Point>();

			 // Points for the new line.
	private: System::Boolean IsDrawing = false;
	private: Point NewPt1;
	private: Point NewPt2;

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~MainForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::PictureBox^  picCanvas;

	

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = gcnew System::ComponentModel::Container();
			this->picCanvas = (gcnew System::Windows::Forms::PictureBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->picCanvas))->BeginInit();
			this->SuspendLayout();
			// 
			// picCanvas
			// 
			this->picCanvas->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom)
				| System::Windows::Forms::AnchorStyles::Left)
				| System::Windows::Forms::AnchorStyles::Right));
			this->picCanvas->Location = System::Drawing::Point(12, 12);
			this->picCanvas->Name = L"picCanvas";
			this->picCanvas->Size = System::Drawing::Size(260, 237);
			this->picCanvas->TabIndex = 0;
			this->picCanvas->TabStop = false;
			this->picCanvas->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MainForm::picCanvas_Paint);
			this->picCanvas->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseDown);
			this->picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDown);
			this->picCanvas->Resize += gcnew System::EventHandler(this, &MainForm::picCanvas_Resize);
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 261);
			this->Controls->Add(this->picCanvas);
			this->Name = L"MainForm";
			this->Text = L"Parcel - Sketch v1";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->picCanvas))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion

		// The mouse is up. See whether we're over an end point or segment.
	private: System::Void picCanvas_MouseMove_NotDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
	{
		auto new_cursor = Cursors::Cross;

		// See what we're over.
		Point hit_point;
		int segment_number;

		if (MouseIsOverEndpoint(e->Location, segment_number, hit_point))
		{
			new_cursor = Cursors::Arrow;
		}
		else if (MouseIsOverSegment(e->Location, segment_number))
		{
			new_cursor = Cursors::Hand;
		}

		// Set the new cursor.
		if (picCanvas->Cursor != new_cursor)
		{
			picCanvas->Cursor = new_cursor;
		}
	}

	private: System::Void picCanvas_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
	{
		// See what we're over.
		Point hit_point;
		int segment_number;

		if (MouseIsOverEndpoint(e->Location, segment_number, hit_point))
		{
			// Start moving this end point.
			picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDown);
			picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_MovingEndPoint);
			picCanvas->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseUp_MovingEndPoint);

			// Remember the segment number.
			MovingSegment = segment_number;

			// See if we're moving the start end point.
			MovingStartEndPoint = (Pt1[segment_number].Equals(hit_point));

			// Remember the offset from the mouse to the point.
			OffsetX = hit_point.X - e->X;
			OffsetY = hit_point.Y - e->Y;
		}
		else if (MouseIsOverSegment(e->Location, segment_number))
		{
			// Start moving this segment.
			picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDown);
			picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_MovingSegment);
			picCanvas->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseUp_MovingSegment);

			// Remember the segment number.
			MovingSegment = segment_number;

			// Remember the offset from the mouse to the segment's first point.
			OffsetX = Pt1[segment_number].X - e->X;
			OffsetY = Pt1[segment_number].Y - e->Y;
		}
		else
		{
			// Start drawing a new segment.
			picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDown);
			picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_Drawing);
			picCanvas->MouseUp += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseUp_Drawing);

			IsDrawing = true;

			int x = e->X;
			int y = e->Y;
			SnapToGrid(x, y);
			NewPt1 = Point(x, y);
			NewPt2 = Point(x, y);
		}
	}


			 // Snap to the nearest grid point.
	private: System::Void SnapToGrid(System::Int32% x, System::Int32% y)
	{
		//if (!chkSnapToGrid->Checked) return;
		x = grid_gap * (System::Int32)Math::Round((System::Double)x / grid_gap);
		y = grid_gap * (System::Int32)Math::Round((System::Double)y / grid_gap);
	}

#pragma region "Drawing"

			 // We're drawing a new segment.
	private: System::Void picCanvas_MouseMove_Drawing(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		// Save the new point.
		System::Int32 x = e->X;
		System::Int32 y = e->Y;
		SnapToGrid(x, y);
		NewPt2 = Point(x, y);

		// Redraw.
		picCanvas->Invalidate();
	}

			 // Stop drawing.
	private: System::Void picCanvas_MouseUp_Drawing(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		IsDrawing = false;

		// Reset the event handlers.
		this->picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_Drawing);
		this->picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDown);
		this->picCanvas->MouseUp -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseUp_Drawing);

		// Create the new segment.
		Pt1->Add(NewPt1);
		Pt2->Add(NewPt2);

		// Redraw.
		this->picCanvas->Invalidate();
	}

#pragma endregion // Drawing

#pragma region "Moving End Point"

			 // The segment we're moving or the segment whose end point we're moving.
	private: System::Int32 MovingSegment = -1;

			 // The end point we're moving.
	private: System::Boolean MovingStartEndPoint = false;

			 // The offset from the mouse to the object being moved.
	private: System::Int32 OffsetX, OffsetY;

			 // We're moving an end point.
	private: System::Void picCanvas_MouseMove_MovingEndPoint(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		// Move the point to its new location.
		System::Int32 x = e->X + OffsetX;
		System::Int32 y = e->Y + OffsetY;
		SnapToGrid(x, y);

		if (MovingStartEndPoint)
			Pt1[MovingSegment] = Point(x, y);
		else
			Pt2[MovingSegment] = Point(x, y);

		// Redraw.
		picCanvas->Invalidate();
	}

			 // Stop moving the end point.
	private: System::Void picCanvas_MouseUp_MovingEndPoint(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		// Reset the event handlers.
		picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDown);
		picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_MovingEndPoint);
		picCanvas->MouseUp -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseUp_MovingEndPoint);

		// Redraw.
		picCanvas->Invalidate();
	}

#pragma endregion // Moving End Point

#pragma region "Moving Segment"

			 // We're moving a segment.
	private: System::Void picCanvas_MouseMove_MovingSegment(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		// See how far the first point will move.
		System::Int32 x = e->X + OffsetX;
		System::Int32 y = e->Y + OffsetY;
		SnapToGrid(x, y);

		System::Int32 dx = x - Pt1[MovingSegment].X;
		System::Int32 dy = y - Pt1[MovingSegment].Y;

		if (dx == 0 && dy == 0) return;

		// Move the segment to its new location.
		Pt1[MovingSegment] = Point(x, y);
		Pt2[MovingSegment] = Point(
			Pt2[MovingSegment].X + dx,
			Pt2[MovingSegment].Y + dy);

		// Redraw.
		picCanvas->Invalidate();
	}

			 // Stop moving the segment.
	private: System::Void picCanvas_MouseUp_MovingSegment(System::Object^ sender, System::Windows::Forms::MouseEventArgs^ e)
	{
		// Reset the event handlers.
		picCanvas->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_NotDown);
		picCanvas->MouseMove -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseMove_MovingSegment);
		picCanvas->MouseUp -= gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::picCanvas_MouseUp_MovingSegment);

		// Redraw.
		picCanvas->Invalidate();
	}

#pragma endregion // Moving End Point


			 // See if the mouse is over an end point.
	private: System::Boolean MouseIsOverEndpoint
			 (Point mouse_pt,
				 [Runtime::InteropServices::Out] System::Int32 %segment_number,
				 [Runtime::InteropServices::Out] Point %hit_pt)
	{
		for (int i = 0; i < Pt1->Count; i++)
		{
			// Check the starting point.
			if (FindDistanceToPointSquared(mouse_pt, Pt1[i]) < over_dist_squared)
			{
				// We're over this point.
				segment_number = i;
				hit_pt = Pt1[i];
				return true;
			}

			// Check the end point.
			if (FindDistanceToPointSquared(mouse_pt, Pt2[i]) < over_dist_squared)
			{
				// We're over this point.
				segment_number = i;
				hit_pt = Pt2[i];
				return true;
			}
		}

		segment_number = -1;
		hit_pt = Point(-1, -1);
		return false;
	}

			 // See if the mouse is over a line segment.
	private: System::Boolean MouseIsOverSegment(Point mouse_pt, [Runtime::InteropServices::Out] System::Int32 %segment_number)
	{
		for (int i = 0; i < Pt1->Count; i++)
		{
			// See if we're over the segment.
			PointF closest;
			if (FindDistanceToSegmentSquared(
				mouse_pt, Pt1[i], Pt2[i], closest)
				< over_dist_squared)
			{
				// We're over this segment.
				segment_number = i;
				return true;
			}
		}

		segment_number = -1;
		return false;
	}

			 // Calculate the distance squared between two points.
	private: System::Int32 FindDistanceToPointSquared(Point pt1, Point pt2)
	{
		int dx = pt1.X - pt2.X;
		int dy = pt1.Y - pt2.Y;
		return dx * dx + dy * dy;
	}

			 // Calculate the distance squared between
			 // point pt and the segment p1 --> p2.
	private: System::Double FindDistanceToSegmentSquared(Point pt, Point p1, Point p2, [Runtime::InteropServices::Out] PointF %closest)
	{
		float dx = p2.X - p1.X;
		float dy = p2.Y - p1.Y;
		if ((dx == 0) && (dy == 0))
		{
			// It's a point not a line segment.
			closest = p1;
			dx = pt.X - p1.X;
			dy = pt.Y - p1.Y;
			return dx * dx + dy * dy;
		}

		// Calculate the t that minimizes the distance.
		float t = ((pt.X - p1.X) * dx + (pt.Y - p1.Y) * dy) / (dx * dx + dy * dy);

		// See if this represents one of the segment's
		// end points or a point in the middle.
		if (t < 0)
		{
			closest = PointF(p1.X, p1.Y);
			dx = pt.X - p1.X;
			dy = pt.Y - p1.Y;
		}
		else if (t > 1)
		{
			closest = PointF(p2.X, p2.Y);
			dx = pt.X - p2.X;
			dy = pt.Y - p2.Y;
		}
		else
		{
			closest = PointF(p1.X + t * dx, p1.Y + t * dy);
			dx = pt.X - closest.X;
			dy = pt.Y - closest.Y;
		}

		return dx * dx + dy * dy;
	}

			 // Draw the lines.
	private: System::Void picCanvas_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e)
	{
		e->Graphics->SmoothingMode = SmoothingMode::AntiAlias;

		// Draw the segments.
		for (int i = 0; i < Pt1->Count; i++)
		{
			// Draw the segment.
			e->Graphics->DrawLine(Pens::Blue, Pt1[i], Pt2[i]);
		}

		// Draw the end points.
		for each(Point pt in Pt1)
		{
			Rectangle rect = Rectangle(
				pt.X - object_radius, pt.Y - object_radius,
				2 * object_radius + 1, 2 * object_radius + 1);
			e->Graphics->FillEllipse(Brushes::White, rect);
			e->Graphics->DrawEllipse(Pens::Black, rect);
		}
		for each(Point pt in Pt2)
		{
			Rectangle rect = Rectangle(
				pt.X - object_radius, pt.Y - object_radius,
				2 * object_radius + 1, 2 * object_radius + 1);
			e->Graphics->FillEllipse(Brushes::White, rect);
			e->Graphics->DrawEllipse(Pens::Black, rect);
		}

		// If there's a new segment under constructions, draw it.
		if (IsDrawing)
		{
			e->Graphics->DrawLine(Pens::Red, NewPt1, NewPt2);
		}
	}

			 // Give the PictureBox a grid background.
	private: System::Void picCanvas_Resize(System::Object^  sender, System::EventArgs^  e)
	{
		this->MakeBackgroundGrid();
	}

	private: System::Void chkSnapToGrid_CheckedChanged(System::Object^  sender, System::EventArgs^  e)
	{
		this->MakeBackgroundGrid();
	}


	private: System::Void MakeBackgroundGrid()
	{

		Bitmap^ bm = gcnew Bitmap(
			picCanvas->ClientSize.Width,
			picCanvas->ClientSize.Height);
		for (int x = 0; x < picCanvas->ClientSize.Width; x += grid_gap)
		{
			for (int y = 0; y < picCanvas->ClientSize.Height; y += grid_gap)
			{
				bm->SetPixel(x, y, Color::Black);
			}
		}

		picCanvas->BackgroundImage = bm;

	}



	private: System::Void  ShowMyImage(String^ fileToDisplay, int xSize, int ySize)
	{
		Bitmap^ MyImage;
		// Sets up an image object to be displayed.
		if (MyImage != nullptr)
		{
			delete MyImage;
		}


		// Stretches the image to fit the pictureBox.
		this->picCanvas->SizeMode = PictureBoxSizeMode::StretchImage;
		MyImage = gcnew Bitmap(fileToDisplay);
		this->picCanvas->ClientSize = System::Drawing::Size(xSize, ySize);
		this->picCanvas->Image = dynamic_cast<Image^>(MyImage);
	}

	private: System::Void  ShowMyImage(Stream^ fileToDisplay, int xSize, int ySize)
	{
		Bitmap^ MyImage;
		// Sets up an image object to be displayed.
		if (MyImage != nullptr)
		{
			delete MyImage;
		}


		// Stretches the image to fit the pictureBox.
		this->picCanvas->SizeMode = PictureBoxSizeMode::StretchImage;
		MyImage = gcnew Bitmap(fileToDisplay);
		this->picCanvas->ClientSize = System::Drawing::Size(xSize, ySize);
		this->picCanvas->Image = dynamic_cast<Image^>(MyImage);
	}


	};
}
