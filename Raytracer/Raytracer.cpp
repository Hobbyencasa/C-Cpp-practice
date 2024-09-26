//The purpose of this file is to learn how to make a basic raytracer and generate some images.

#define global_variable static
#define internal static

#define COLOR_RED (3 << 22)
#define COLOR_BLUE (3 << 6)
#define COLOR_GREEN (3 << 14)
#define BIGNUMBA 10000.f
#define MAX_SPHERES 5
#define MAX_LIGHTS 5
#define SPHERE_SPEED 0.1f

#include <windows.h>
#include <stdint.h>
#include <math.h>

typedef struct cartesian_vector
{
  float x;
  float y;
  float z;
}cartesian_vector;

typedef union light_vector
{
  cartesian_vector direction;
  cartesian_vector position;
  int ambient;
}light_vector;

typedef struct light
{
  uint8_t type; //1 for ambient, 2 for point, 3 for directional
  float intensity;
  light_vector vector; //if it's ambient, I will set its integer value to 0.
}light;

typedef struct sphere
{
  cartesian_vector center;
  float radius;
  DWORD color;
  int specular;
  float reflective;
}sphere;

typedef struct intersector
{//To be used in ClosestIntersection for shadows. Check book, shadow part. Gave it a cool name just for the sake of it.
  sphere interSphere;
  float interT;
}intersector;

typedef struct eq_solution
{
  float t1;
  float t2;
}eq_solution;

typedef struct pixel
{
  int x;
  int y;
  DWORD color;
}pixel;

typedef struct window_dimension
{
  int width;
  int height;
}window_dimension;

typedef struct screen_buffer
{
  void * memory;
  BITMAPINFO info;
  int width;
  int height;
  int bytesPerPixel;
  int pitch;
}screen_buffer;

//Declaring globals
global_variable screen_buffer GlobalBuffer;
global_variable cartesian_vector GlobalOrigin;
global_variable sphere * GlobalSphereHolder[MAX_SPHERES+1];
global_variable sphere TestSphere1;
global_variable	sphere TestSphere2;
global_variable	sphere TestSphere3;
global_variable	sphere TestSphere4;
global_variable light * GlobalLightHolder[MAX_LIGHTS+1];
global_variable light AmbientLight;
global_variable light PointLight1;
global_variable light DirectionalLight1;

internal window_dimension WinGetWindowDimension(HWND Window)
{
  RECT ClientRect;
  window_dimension Result;
  GetClientRect(Window, &ClientRect);
  Result.width = ClientRect.right - ClientRect.left;
  Result.height = ClientRect.bottom - ClientRect.top;
  return Result;
}

internal void WinAdjustBuffer(screen_buffer * Buffer, int width, int height)
{
  if ((*Buffer).memory)
    {
      VirtualFree((*Buffer).memory,0,MEM_RELEASE);
    }
  (*Buffer).width = width;
  (*Buffer).height = height;
  (*Buffer).bytesPerPixel = 4;
  (*Buffer).pitch = (*Buffer).bytesPerPixel * width;
  (*Buffer).info.bmiHeader.biSize = sizeof((*Buffer).info.bmiHeader) ;
  (*Buffer).info.bmiHeader.biWidth = width ;
  (*Buffer).info.bmiHeader.biHeight = -height;
  (*Buffer).info.bmiHeader.biPlanes = 1;
  (*Buffer).info.bmiHeader.biBitCount = 32;
  (*Buffer).info.bmiHeader.biCompression = BI_RGB;
  
  int BitmapMemorySize = (*Buffer).bytesPerPixel * (*Buffer).width * (*Buffer).height;
  (*Buffer).memory = VirtualAlloc(0,BitmapMemorySize,MEM_COMMIT, PAGE_READWRITE);
}

internal void WinDrawToScreenBuffer(screen_buffer * Buffer, int x, int y, DWORD Color)
{
  uint8_t * Row = (uint8_t *)(*Buffer).memory;
  Row += (*Buffer).pitch * y;
  uint32_t * Pixel = (uint32_t *)Row;
  Pixel += x;
  *Pixel = Color;
}

internal void WinDrawFromBufferToScreen(HDC DeviceContext,screen_buffer Buffer)
{
  StretchDIBits(DeviceContext,0,0,Buffer.width,Buffer.height,0,0,Buffer.width,Buffer.height, Buffer.memory, &Buffer.info, DIB_RGB_COLORS, SRCCOPY);
}

internal cartesian_vector CanvasToViewport(int ViewportWidth, int ViewportHeight, int CanvasWidth, int CanvasHeight,int x, int y, int distance)
{
  cartesian_vector result;
  result.x = ((float)ViewportWidth * (float)x)/((float)CanvasWidth);
  result.y = ((float)ViewportHeight * (float)y)/((float)CanvasHeight);
  result.z = (float)distance;
  
  return result;
}

internal eq_solution IntersectRayToSphere(cartesian_vector O, cartesian_vector D, sphere Sphere)
{
  float r = Sphere.radius;
  cartesian_vector CenterToOrigin;
  CenterToOrigin.x = O.x - Sphere.center.x;
  CenterToOrigin.y = O.y - Sphere.center.y;
  CenterToOrigin.z = O.z - Sphere.center.z;

  float a = (D.x * D.x) + (D.y * D.y) + (D.z * D.z);
  float b = 2*( (CenterToOrigin.x * D.x)+ (CenterToOrigin.y * D.y)+ (CenterToOrigin.z * D.z));
  float c = ((CenterToOrigin.x*CenterToOrigin.x )+(CenterToOrigin.y*CenterToOrigin.y )+(CenterToOrigin.z*CenterToOrigin.z )) - r*r;

  float discriminant = b*b - 4*a*c;
  eq_solution Result;

  if (discriminant < 0)
    {
      Result.t1 = BIGNUMBA;
      Result.t2 = BIGNUMBA;
      return Result;
    }
  Result.t1 = (-b + sqrtf(discriminant)) / (2*a);
  Result.t2 = (-b - sqrtf(discriminant)) / (2*a);
  
  return Result;
}

internal float DotProduct(cartesian_vector V1, cartesian_vector V2)
{
  return ((V1.x * V2.x) + (V1.y * V2.y) + (V1.z * V2.z));
}

internal DWORD CrampColor(DWORD Color, float multiplier)
{
  uint8_t RedChannel = (uint8_t)(Color >> 16);
  uint8_t GreenChannel = (uint8_t)(Color >> 8);
  uint8_t BlueChannel = (uint8_t) Color;
  if (RedChannel * multiplier > 255)
    {
      RedChannel = 255;
    }
  else
    {
      RedChannel = RedChannel * multiplier;
    }
  
  if (GreenChannel * multiplier > 255)
    {
      GreenChannel = 255;
    }
  else
    {
      GreenChannel = GreenChannel * multiplier;
    }
  
  if (BlueChannel * multiplier > 255)
    {
      BlueChannel = 255;
    }
  else
    {
      BlueChannel = BlueChannel * multiplier;
    }
  DWORD LocalColor = 0 | (RedChannel <<16) | (GreenChannel << 8) | (BlueChannel) ;
  return LocalColor;
}

internal DWORD AddColors(DWORD Color1, DWORD Color2)
{
  uint8_t Red1 = (uint8_t)(Color1 >> 16);
  uint8_t Red2 = (uint8_t)(Color2 >> 16);
  uint8_t Green1 = (uint8_t)(Color1 >> 8);
  uint8_t Green2 = (uint8_t)(Color2 >> 8);
  uint8_t Blue1 = (uint8_t)Color1;
  uint8_t Blue2 = (uint8_t)Color2;
  uint8_t RedTotal;
  uint8_t GreenTotal;
  uint8_t BlueTotal;
  (Red1 + Red2 > 255) ? RedTotal = 255 : RedTotal = Red1+Red2;
  (Green1 + Green2 > 255) ? GreenTotal = 255 : GreenTotal = Green1+Green2;
  (Blue1 + Blue2 > 255) ? BlueTotal = 255 : BlueTotal = Blue1+Blue2;
  DWORD Result = 0 | (RedTotal << 16) | (GreenTotal << 8) | (BlueTotal) ;
  return Result;
}

internal cartesian_vector ReflectRay(cartesian_vector R, cartesian_vector N)
{
  cartesian_vector Result;
  float R_dot_N = DotProduct(R,N);
  Result.x = 2*N.x * R_dot_N - R.x;
  Result.y = 2*N.y * R_dot_N - R.y;
  Result.z = 2*N.z * R_dot_N - R.z;
  return Result;
}

internal intersector ClosestIntersection(cartesian_vector O, cartesian_vector D, float tMin, float tMax)
{
  intersector Result;
  float ClosestT = BIGNUMBA;
  sphere ClosestSphere = {};
  eq_solution Solution = {};

  for (int i = 0; i < MAX_SPHERES; ++i)
    {
      if(GlobalSphereHolder[i])
	{
	  Solution = IntersectRayToSphere(O,D,(*GlobalSphereHolder[i]));
	  if( (Solution.t1 <= tMax ) && (Solution.t1 >= tMin) && (Solution.t1 < ClosestT) )
	    {
	      ClosestT = Solution.t1;
	      ClosestSphere = (*GlobalSphereHolder[i]);
	    }
	  if( (Solution.t2 <= tMax ) && (Solution.t2 >= tMin) && (Solution.t2 < ClosestT) )
	    {
	      ClosestT = Solution.t2;
	      ClosestSphere = (*GlobalSphereHolder[i]);
	    }
	}
    }
  Result.interSphere = ClosestSphere;
  Result.interT = ClosestT;
  
  return Result;
}

internal float ComputeLighting(cartesian_vector P, cartesian_vector N, cartesian_vector V, int specular)
{
  float Intensity = 0.f;
  for (int counter = 0; counter < MAX_LIGHTS; ++counter)
    {
      if(GlobalLightHolder[counter])
	{
	  if((*GlobalLightHolder[counter]).type == 1)//ambient light
	    {
	      Intensity += (*GlobalLightHolder[counter]).intensity;
	    }
	  else
	    {
	      cartesian_vector L;
	      float tMax;
	      if ((*GlobalLightHolder[counter]).type == 2)//point light
		{
		  L.x = (*GlobalLightHolder[counter]).vector.position.x - P.x;
		  L.y = (*GlobalLightHolder[counter]).vector.position.y - P.y;
		  L.z = (*GlobalLightHolder[counter]).vector.position.z - P.z;
		  tMax = 1.f;
		}
	      else//directional light
		{
		  L = (*GlobalLightHolder[counter]).vector.direction;
		  tMax = BIGNUMBA;
		}
	      //Shadow check
	      intersector ShadowIntersector = ClosestIntersection(P, L, 0.001, tMax);
	      if (ShadowIntersector.interSphere.radius != 0)
		{
		  continue;
		}

	      //Diffuse reflection
	      float N_dot_L = DotProduct(N,L);
	      if (N_dot_L > 0)
		{
		  float LengthL = sqrtf(DotProduct(L,L));
		  float LengthN = sqrtf(DotProduct(N,N)); //This should be 1. Can this line be removed without causing problems?
		  Intensity += (*GlobalLightHolder[counter]).intensity * N_dot_L/(LengthL*LengthN);//Can LengthN be replaced by 1.f here?
		}

	      //Specular reflection
	      if (specular != -1)
		{
		  cartesian_vector R = ReflectRay(L,N);
		  float LengthR = sqrtf(DotProduct(R,R));
		  float LengthV = sqrtf(DotProduct(V,V));

		  float R_dot_V = DotProduct(R,V);
		  if (R_dot_V > 0)
		    {
		      Intensity += (*GlobalLightHolder[counter]).intensity * powf((R_dot_V/(LengthR * LengthV)) , specular);
		    }
		}
	    }
	}
    }
  return Intensity;
}

internal DWORD TraceRay(cartesian_vector O, cartesian_vector D, float tMin, float tMax, int recursion_depth)
{
  intersector Intersector = ClosestIntersection(O,D,tMin,tMax);
  sphere ClosestSphere = Intersector.interSphere;
  float ClosestT = Intersector.interT;
  
  if (!ClosestSphere.radius)
    {
      return 0xffffff;
    }
  
  cartesian_vector P;
  P.x = O.x + ClosestT*D.x;
  P.y = O.y + ClosestT*D.y;
  P.z = O.z + ClosestT*D.z;
  cartesian_vector N;
  N.x = P.x - ClosestSphere.center.x;
  N.y = P.y - ClosestSphere.center.y;
  N.z = P.z - ClosestSphere.center.z;
  float LengthN = sqrtf((N.x*N.x) + (N.y*N.y) + (N.z*N.z));
  N.x /= LengthN;
  N.y /= LengthN;
  N.z /= LengthN;
  cartesian_vector V;
  V.x = -D.x;
  V.y = -D.y;
  V.z = -D.z;
  float Intensity = ComputeLighting(P,N,V,ClosestSphere.specular);
  DWORD LocalColor = CrampColor(ClosestSphere.color, Intensity);

  float reflective = ClosestSphere.reflective;
  if( (recursion_depth <=0) || (reflective <= 0))
    {
      return LocalColor;
    }

  cartesian_vector R = ReflectRay(V,N);
  DWORD ReflectedColor = TraceRay(P,R,0.001,BIGNUMBA, recursion_depth - 1);

  DWORD ReturnColorLocal = CrampColor(LocalColor, 1-reflective);
  DWORD ReturnColorReflected = CrampColor(ReflectedColor, reflective);
  DWORD Result = AddColors(ReturnColorLocal, ReturnColorReflected);
  
  return Result;
}

LRESULT WindowProcedure(HWND WindowHandle, UINT Message, WPARAM wParam, LPARAM lParam)
{
  return DefWindowProcA(WindowHandle, Message, wParam, lParam);
}

int WINAPI WinMain( HINSTANCE Instance,
		    HINSTANCE PrevInstance,
		    LPSTR lpCmdLine,
		    int nShowCmd)
{
  bool Running = false;
  WNDCLASSA WindowClass = {};
  WindowClass.style = CS_VREDRAW | CS_HREDRAW;
  WindowClass.lpfnWndProc = WindowProcedure;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "RaytracerWindowClass";

  if (RegisterClassA(&WindowClass))
    {
      HWND Window = CreateWindowA(WindowClass.lpszClassName, "Raytracer Window",WS_VISIBLE | WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,0,0,Instance,0);

      if (Window)
	{
	  window_dimension WindowDimension = WinGetWindowDimension(Window);
	  WinAdjustBuffer(&GlobalBuffer,WindowDimension.width, WindowDimension.height);
	  Running = true;
	  HDC DeviceContext = GetDC(Window);

	  //Set the Origin
	  GlobalOrigin.x = 0.f;
	  GlobalOrigin.y = 0.f;
	  GlobalOrigin.z = 0.f;

	  //Initializing spheres
	  TestSphere1.center.x = 0.f;
	  TestSphere1.center.y = -1.f;
	  TestSphere1.center.z = 3.f;
	  TestSphere1.radius = 1.f;
	  TestSphere1.specular = 5;
	  TestSphere1.reflective = 0.2f;
	  TestSphere1.color = COLOR_RED;
	  TestSphere2.center.x = 2.f;
	  TestSphere2.center.y = 0.f;
	  TestSphere2.center.z = 4.f;
	  TestSphere2.radius = 1.f;
	  TestSphere2.specular = 5;
	  TestSphere2.reflective = 0.3f;
	  TestSphere2.color = COLOR_BLUE;
	  TestSphere3.center.x = -2.f;
	  TestSphere3.center.y = 0.f;
	  TestSphere3.center.z = 4.f;
	  TestSphere3.radius = 1.f;
	  TestSphere3.specular = 10;
	  TestSphere3.reflective = 0.7f;
	  TestSphere3.color = COLOR_GREEN;
	  TestSphere4.center.x = 0.f ;
	  TestSphere4.center.y = -501.f;
	  TestSphere4.center.z = 0.f ;
	  TestSphere4.radius = 500;
	  TestSphere4.specular = 100;
	  TestSphere4.reflective = 0.1f;
	  TestSphere4.color = 0xffff00;
	  GlobalSphereHolder[0] = &TestSphere1;
	  GlobalSphereHolder[1] = &TestSphere2;
	  GlobalSphereHolder[2] = &TestSphere3;
	  GlobalSphereHolder[3] = &TestSphere4;

	  //Set the light sources
	  AmbientLight.type = 1;
	  AmbientLight.intensity =0.2f;
	  AmbientLight.vector.ambient = 0;
	  PointLight1.type = 2;
	  PointLight1.intensity = 0.6f;
	  PointLight1.vector.position = {2.f, 1.f, 0.f};
	  DirectionalLight1.type = 3;
	  DirectionalLight1.intensity = 0.2f;
	  DirectionalLight1.vector.direction = {1.f, 4.f, 4.f};
	  GlobalLightHolder[0] = &AmbientLight;
	  GlobalLightHolder[1] = &PointLight1;
	  GlobalLightHolder[2] = &DirectionalLight1;
	  
	  //Set the viewport
	  int ProjectionPlaneDistance = 1;
	  window_dimension ViewportSize;
	  ViewportSize.width = 2;
	  ViewportSize.height = 1;

	  //Recursion limit for reflections
	  int recursion_depth = 2;
	  
	  MSG Message = {};
	  while(Running)
	    {

	      for (int x = (-WindowDimension.width/2); x <= (WindowDimension.width/2); ++x)
		{
		  for (int y = (-WindowDimension.height/2)+2; y <= (WindowDimension.height/2); ++y)
		    {
		      cartesian_vector D = CanvasToViewport(ViewportSize.width,ViewportSize.height,WindowDimension.width,WindowDimension.height,x,y,ProjectionPlaneDistance);
		      DWORD color = TraceRay(GlobalOrigin, D, 1.f, BIGNUMBA, recursion_depth);
		      int ActualX = x + (WindowDimension.width/2);
		      int ActualY = (WindowDimension.height/2)- y;
		      WinDrawToScreenBuffer(&GlobalBuffer,ActualX,ActualY,color);
		    }
		}
	      	WinDrawFromBufferToScreen(DeviceContext, GlobalBuffer);
		  
	      if(PeekMessage(&Message, Window,0,0,PM_REMOVE))
		{
		  if(Message.wParam == 'Q')
		    {
		      Running = false;
		    }
		  if(Message.wParam == 'W')
		    {
		      TestSphere1.center.z += SPHERE_SPEED;
		    }
		  if(Message.wParam == 'S')
		    {
		      TestSphere1.center.z -= SPHERE_SPEED;
		    }
		  if(Message.wParam == 'D')
		    {
		      TestSphere1.center.x += SPHERE_SPEED;
		    }
		  if(Message.wParam == 'A')
		    {
		      TestSphere1.center.x -= SPHERE_SPEED;
		    }
		  if(Message.wParam == 'T')
		    {
		      TestSphere1.center.y += SPHERE_SPEED;
		    }
		  if(Message.wParam == 'R')
		    {
		      TestSphere1.center.y -= SPHERE_SPEED;
		    }
		  
		}
	      
	    }//End of WHILE(RUNNING)
	  
	}//End of IF (WINDOW)
      
    }//End of IF (REGISTERCLASSA (WINDOWCLASS))

  

  return 0;
}
