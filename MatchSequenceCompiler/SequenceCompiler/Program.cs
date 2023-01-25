using System;
using System.Collections.Generic;

namespace SequenceCompiler
{
    class Program
    {
        const int max_name_length = 15;
        enum StageAction { Nothing, TargetsFacing, TargetsAway };
        struct Stage
        {
            public String name;
            public TimeSpan duration;
            public StageAction start;
            public StageAction stop;
        }
        struct RangeProgram
        {
            public String program_name;
            public List<Stage> stages;
        }
        static void Main(string[] args)
        {
            int match_number = 0;
            List<RangeProgram> programs = new List<RangeProgram>();
            do
            {
                RangeProgram program_design = new RangeProgram();
                Console.WriteLine(String.Format("Enter the name of the program for spot {0}. [Blank = No more matches]", match_number + 1));
                string match_name = Console.ReadLine();
                if (match_name.Length == 0)
                {
                    break;
                }
                program_design.program_name = match_name.Substring(0, Math.Min(match_name.Length, max_name_length));
                program_design.stages = new List<Stage>();
                int stage_number = 0;
                do
                {
                    Stage stage_design = new Stage();
                    Console.WriteLine(String.Format("Enter the name of stage {0}. [Blank = no more stages]", stage_number + 1));
                    string stage_name = Console.ReadLine();
                    if (stage_name.Length == 0)
                    {
                        break;
                    }
                    stage_design.name = stage_name.Substring(0, Math.Min(stage_name.Length, max_name_length));
                    Console.WriteLine("How long does the stage last? [mm:ss] ex. 1:30 for one minute and 30 seconds");
                    String duration_as_string = Console.ReadLine();
                    int colon_location = duration_as_string.IndexOf(':');
                    int minutes = 0;
                    int seconds = 0;
                    if (colon_location >= 0)
                    {
                        minutes = int.Parse(duration_as_string.Substring(0, colon_location));
                        seconds = int.Parse(duration_as_string.Substring(colon_location + 1));
                    }
                    else
                    {
                        seconds = int.Parse(duration_as_string);
                        minutes = seconds / 60;
                        seconds = seconds % 60;
                    }
                    stage_design.duration = new TimeSpan(0, minutes, seconds);
                    Console.WriteLine("What action at the start of this stage? (0 - [N]othing, 1 - [F]acing, 2 - [A]way)");
                    string action = Console.ReadLine();
                    ParseAction(action, out stage_design.start);
                    Console.WriteLine("What action at the end of this stage? (0 - [N]othing, 1 - [F]acing, 2 - [A]way)");
                    action = Console.ReadLine();
                    ParseAction(action, out stage_design.stop);

                    program_design.stages.Add(stage_design);
                    stage_number++;
                } while (stage_number < 5);
                programs.Add(program_design);
                match_number++;
            } while (match_number < 15);

            byte[] data = new byte[1024];
            Array.Fill<byte>(data, 0);
            int current_program_offset = 0x20;
            int current_program_index = 0;
            foreach (RangeProgram program in programs)
            {
                int current_position = current_program_offset + 1;
                int current_name_pos = 0;
                foreach (char name in program.program_name)
                {
                    data[current_position + current_name_pos] = (byte)name;
                    current_name_pos++;
                }
                current_position += max_name_length + 1;
                data[current_position++] = (byte)program.stages.Count;
                foreach (Stage stage in program.stages)
                {
                    current_name_pos = 0;
                    foreach (char name in stage.name)
                    {
                        data[current_position + current_name_pos] = (byte)name;
                        current_name_pos++;
                    }
                    current_position += max_name_length + 1;
                    data[current_position++] = (byte)stage.duration.Minutes;
                    data[current_position++] = (byte)stage.duration.Seconds;
                    data[current_position++] = (byte)(((byte)stage.start) << 4 | ((byte)stage.stop));
                }
                data[current_program_offset] = (byte)(current_position - current_program_offset);
                data[current_program_index * 2] = (byte)((current_program_offset >> 8) & 0xFF);
                data[current_program_index * 2 + 1] = (byte)(current_program_offset & 0xFF);
                current_program_offset = current_position;
                current_program_index++;
            }

            int counter = 0;
            string current_line = "__EEPROM_DATA(";
            foreach (byte value in data)
            {
                if (counter == 0)
                {
                    current_line += String.Format("0x{0:X2}", value);
                    counter++;
                }
                else if (counter != 7)
                {
                    current_line += String.Format(", 0x{0:X2}", value);
                    counter++;
                }
                else
                {
                    counter = 0;
                    current_line += String.Format(", 0x{0:X2});", value);
                    Console.WriteLine(current_line);
                    current_line = "__EEPROM_DATA(";
                }
            }

            if (counter != 0)
            {
                while (counter != 8)
                {
                    current_line += ", 0x00";
                    counter++;
                }
                current_line += ");";
                Console.WriteLine(current_line);
            }
            Console.WriteLine("Complete, press any key to close");
            Console.ReadKey();
        }

        private static void ParseAction(string action_string, out StageAction action)
        {
            int action_value = 0;
            if (int.TryParse(action_string, out action_value))
            {
                action = (StageAction)action_value;
            }
            else
            {
                if (action_string.ToUpper() == "NOTHING" || action_string.ToUpper() == "N")
                {
                    action = StageAction.Nothing;
                }
                else if (action_string.ToUpper() == "FACING" || action_string.ToUpper() == "F")
                {
                    action = StageAction.TargetsFacing;
                }
                else if (action_string.ToUpper() == "AWAY" || action_string.ToUpper() == "A")
                {
                    action = StageAction.TargetsAway;
                }
                else
                {
                    Console.WriteLine("Invalid value, using Nothing as action.");
                    action = StageAction.Nothing;
                }
            }
        }
    }
}
