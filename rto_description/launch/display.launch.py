import os
import subprocess

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    pkg_share = get_package_share_directory('rto_description')
    default_model_path = os.path.join(pkg_share, 'urdf', 'robots', 'rto-3.urdf.xacro')
    default_rviz_config_path = os.path.join(pkg_share, 'conf', 'display.rviz')

    model_arg = DeclareLaunchArgument(
        name='model',
        default_value=default_model_path,
        description='Absolute path to robot xacro/urdf file',
    )
    rviz_arg = DeclareLaunchArgument(
        name='rvizconfig',
        default_value=default_rviz_config_path,
        description='Absolute path to rviz config file',
    )
    gui_arg = DeclareLaunchArgument(
        name='gui',
        default_value='true',
        choices=['true', 'false'],
        description='Flag to enable joint_state_publisher_gui',
    )

    def launch_nodes(context):
        model_path = context.perform_substitution(
            __import__('launch').substitutions.LaunchConfiguration('model')
        )
        rviz_config = context.perform_substitution(
            __import__('launch').substitutions.LaunchConfiguration('rvizconfig')
        )
        use_gui = context.perform_substitution(
            __import__('launch').substitutions.LaunchConfiguration('gui')
        )

        # Process xacro if needed, otherwise read URDF directly
        if model_path.endswith('.xacro'):
            robot_description = subprocess.check_output(['xacro', model_path], text=True)
        else:
            with open(model_path, 'r') as f:
                robot_description = f.read()

        nodes = [
            Node(
                package='robot_state_publisher',
                executable='robot_state_publisher',
                parameters=[{'robot_description': robot_description}],
            ),
            Node(
                package='rviz2',
                executable='rviz2',
                arguments=['-d', rviz_config],
                output='screen',
            ),
        ]

        # Add joint_state_publisher_gui if available and requested
        if use_gui == 'true':
            try:
                get_package_share_directory('joint_state_publisher_gui')
                nodes.append(Node(
                    package='joint_state_publisher_gui',
                    executable='joint_state_publisher_gui',
                ))
            except Exception:
                pass

        return nodes

    return LaunchDescription([
        model_arg,
        rviz_arg,
        gui_arg,
        OpaqueFunction(function=launch_nodes),
    ])
